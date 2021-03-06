<?php
/**
 * TripalVocabController extends DrupalDefaultEntityController.
 *
 * Our subclass of DrupalDefaultEntityController lets us add a few
 * important create, update, and delete methods.
 */
class TripalVocabController extends EntityAPIController {

  public function __construct($entity_type) {
    parent::__construct($entity_type);
  }

  public function create(array $values = array()) {
    $this->vocabulary = array_key_exists('vocabulary', $values) ? $values['vocabulary'] : '';
    return parent::create($values);
  }

  /**
   * Delete a single entity.
   *
   * Really a convenience function for deleteMultiple().
   */
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $entities = $ids ? $this->load($ids) : FALSE;
    if (!$entities) {
      // Do nothing, in case invalid or no ids have been passed.
      return;
    }
    $transaction = isset($transaction) ? $transaction : db_transaction();
    try {
      $ids = array_keys($entities);
      foreach ($entities as $id => $entity) {
        // Invoke hook_entity_delete().
        module_invoke_all('entity_delete', $entity, 'TripalTerm');
        field_attach_delete('TripalVocab', $entity);
      }

      db_delete('tripal_term')
        ->condition('id', $ids)
        ->execute();
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('tripal', $e);
      throw $e;
      return FALSE;
    }
    return TRUE;
  }

  /**
   * Saves the custom fields using drupal_write_record().
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    global $user;
    $pkeys = array();

    $transaction = isset($transaction) ? $transaction : db_transaction();
    try {
      // If our entity has no id, then we need to give it a
      // time of creation.
      if (empty($entity->id)) {
        $entity->created = time();
        $invocation = 'entity_insert';
      }
      else {
        $invocation = 'entity_update';
        $pkeys = array('id');
      }

      // Invoke hook_entity_presave().
      module_invoke_all('entity_presave', $entity, 'TripalTerm');

      // Write out the entity record.
      $record = array(
        'vocabulary' => $entity->vocabulary,
        'created' => $entity->created,
        'changed' => time(),
      );
      if ($invocation == 'entity_update') {
        $record['id'] = $entity->id;
      }
      $success = drupal_write_record('tripal_vocab', $record, $pkeys);
      if ($success == SAVED_NEW) {
        $entity->id = $record['id'];
      }

      // Now we need to either insert or update the fields which are
      // attached to this entity. We use the same primary_keys logic
      // to determine whether to update or insert, and which hook we
      // need to invoke.
      if ($invocation == 'entity_insert') {
        field_attach_insert('TripalVocab', $entity);
      }
      else {
        field_attach_update('TripalVocab', $entity);
      }

      // Invoke either hook_entity_update() or hook_entity_insert().
      module_invoke_all('entity_postsave', $entity, 'TripalTerm');
      module_invoke_all($invocation, $entity, 'TripalTerm');

      return $record;
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('tripal_entity', $e);
      drupal_set_message("Could not save the TripalVocab:" . $e->getMessage(), "error");
      return FALSE;
    }
  }
}