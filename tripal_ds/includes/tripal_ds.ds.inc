<?php


/**
 * Sorts a multidimensional array into alphabetical order.
 *
 * @param $key.
 *
 * @return \Closure
 */
function sort_array($key) {
  return function ($a, $b) use ($key) {
    return strnatcmp($a[$key], $b[$key]);
  };
}

/**
 * Sorts a multidimensional array of objects into alphabetical order.
 *
 * @param $key.
 *
 * @return \Closure
 */
function sort_object($key) {
  return function ($a, $b) use ($key) {
    return strnatcmp($a->$key, $b->$key);
  };
}
/**
 * Implements hook_ds_layout_settings_info().
 */
function _ds_layout_settings_info($bundle_name, $instances) {
  $region_right = array();
  $region_left = array();
  $prop_fields = array();
  $summary_fields = array();
  $data_sequence_fields = array();
  $all_other_fields = array();
  $fields_with_regions = array();
  $i = 0;
  $all_fields = array();

  try {
    // Get the bundle and term objects.
    $bundle = tripal_load_bundle_entity(array('name' => $bundle_name));
    $term = tripal_load_term_entity(array('term_id' => $bundle->term_id));

    // Build one large multidimensional array of all instances to sort in alpha
    // order to display fields in label alpha order.
    foreach ($instances as $key => $instance) {
      $all_fields[$i] = $instance;
      $i++;
    }
    usort($all_fields, sort_array('label'));

    // Iterate through the fields of this bundle.
    foreach ($all_fields as $key => $instance){
      $instance_name = $instance['field_name'];
      if($instance_name=="rdfs__type"){
        array_push($summary_fields, $instance_name);
      }
      else {
        //TODO: How do we handle non-chado dbs, placement of fields within
        // tripal panes might need to be done in a hook.
        $instance_base_table = array_key_exists('base_table', $instance['settings']) ? $instance['settings']['base_table'] : '';
        $instance_base_chado = array_key_exists('chado_table', $instance['settings']) ? $instance['settings']['chado_table'] : '';
        $prop_table = strpos($instance_base_chado, 'prop');
        $data_sequence = strpos($instance_name, 'data__sequence');
        if ($instance_base_chado && $instance_base_table){

          if ($instance_base_chado == $instance_base_table){
            if ($prop_table !== FALSE){
              array_push($prop_fields, $instance_name);
            }
            elseif ($data_sequence !== FALSE) {
              array_push($data_sequence_fields, $instance_name);
            }
            else {
              array_push($summary_fields, $instance_name);}

          }
          elseif ($instance_base_chado != $instance_base_table){
            if ($prop_table !== FALSE){
              array_push($prop_fields, $instance_name);
            }
            elseif ($data_sequence !== FALSE){
              array_push($data_sequence_fields, $instance_name);
            }
            else {
              array_push($all_other_fields, $instance);

              // Update the display settings so that the title is hidden.
              $instance['display']['default']['label'] = 'hidden';
              field_update_instance($instance);
            }
          }
        }
        else {
          // The tripal_chado module adds an image to the organism content
          // type so we want to make sure that image goes in the summary.
          // It is not a TripalField so it won't have a chado table.
          if ($instance_name == 'data__image' and $term->name == 'organism') {
            array_push($summary_fields, $instance_name);
          }
        }
      }
    }

    // Consolidate the field sets.
    if(!empty($summary_fields)){
      _summary_field_group_info($bundle_name, $summary_fields);
    }
    if (!empty($prop_fields)){
      _prop_field_group_info($bundle_name, $prop_fields);
    }
    if (!empty($data_sequence_fields)){
      _data_sequence_field_group_info($bundle_name, $data_sequence_fields);
    }
    if (!empty($all_other_fields)){
      foreach ($all_other_fields as $key => $other_field) {
        $group_field_name = 'gp_'.$other_field['field_name'];

        // Need to truncate the names because of database field size restrictions,
        // updating fields here to ensure name consistency.
        $group_field_name = substr($group_field_name, 0, 27);

        // Add random numbers to ensure the field name is unique within the 32
        // character limit of the field.
        $group_field_name = $group_field_name.rand(0, 99999);
        _additional_fields_field_group_info($bundle_name, $other_field['label'], $group_field_name, $other_field['field_name']);
      }
    }

    // Build one large multidimensional array of all instances to sort in alpha
    // order to display fields in label alpha order.
    $all_field_groups = field_group_info_groups('TripalEntity', $bundle_name);
    foreach ($all_field_groups['default'] as $key => $field_name) {
      $right_fields[$key] = $field_name;
    }
    usort($right_fields, sort_object('label'));

    // Now build the $region_right array and the fields array.
    $i = 0;
    foreach ($right_fields as $index => $field) {
      // Check if the child is already present which is a problem when groups
      // are nested within groups.
      if (in_array($field->group_name, $region_right)) {
        // Still need to check for children and add them.
        if (!empty($field->children)) {
          foreach($field->children as $index => $child){
            $region_right[$i] = $child;
            $i++;
          }
        }
      }
      else {
        $region_right[$i] = $field->group_name;
        if (!empty($field->children)) {
          foreach ($field->children as $index => $child) {
            $i++;
            $region_right[$i] = $child;
          }
        }
        $i++;
      }
      // Now update the weights of the field_groups.
      tripal_ds_field_group_update_weight($field->group_name, $bundle_name, $i);
    }
    foreach($region_right as $index => $field){
      $fields_with_regions[$field] = 'right';
    }
    // Add blocks to $region_left and build the toc field that is placed within.
    _ds_fields_info_write($bundle_name);
    $region_left += [ 'toc' ];
    $fields_with_regions += [ 'toc' => 'left' ];
    // Build the ds layout.
    $record = new stdClass;
    $record->id ='TripalEntity|' . $bundle_name . '|default';
    $record->entity_type = 'TripalEntity';
    $record->bundle = $bundle_name;
    $record->view_mode = 'default';
    $record->layout = 'tripal_ds_feature';
    $settings = array(
      'regions' => array(
        'left' =>
          $region_left,
        'right' =>
          $region_right,
      ),
      'fields' =>
        $fields_with_regions,
      'classes' => array(),
      'wrappers' => array(
        'left' => 'div',
        'right' => 'div',
      ),
      'layout_wrapper' => 'div',
      'layout_attributes' => '',
      'layout_attributes_merge' => 1,
      'layout_link_attribute' => '',
      'layout_link_custom' => '',
      'layout_disable_css' => 0,
    );
    $record->settings = $settings;
    drupal_write_record('ds_layout_settings', $record);
  }
  catch (Exception $e) {
    watchdog_exception('tripal_ds', $e);
    return FALSE;
  }
  return TRUE;
}


/**
 *  Implements hook_ds_layout_settings_info().
 */
function _ds_layout_pub_settings_info($bundle_name, $instances) {
  $region_right = array();
  $region_left = array();
  $properties= array();
  $all_fields = array();
  $instances_for_field_groups = array();
  $disabled_instances = array();

  try {
    // Add Abstract, Citation, DB Cross Reference, Properties.
    $all_fields['tpub__abstract']= 'right';
    $all_fields['tpub__citation']= 'right';
    $all_fields['sbo__database_cross_reference']= 'right';
    $all_fields['tpub__publication_type']= 'right';
    $all_fields['tpub__doi']= 'right';
    $all_fields['tpub__publication_date']= 'right';
    $all_fields['sio__references']= 'right';

    // Iterate through the fields of this bundle.
    foreach ($instances as $key => $instance) {
      $instance_name = $instance['field_name'];

      if($instance_name == 'tpub__abstract' || $instance_name == 'tpub__citation' || $instance_name == 'sbo__database_cross_reference' || $instance_name == 'sio__references'){
        array_push($instances_for_field_groups, $instance);
        // Update the display settings so that the title is hidden.
        $instance['display']['default']['label'] = 'hidden';
        field_update_instance($instance);
      }
      elseif($instance_name == 'tpub__publication_type' || $instance_name == 'tpub__doi' || $instance_name == 'tpub__publication_date') {
        array_push($properties, $instance);
      }
      else {
        array_push($disabled_instances, $instance);
      }

    }
    //Publication fields that are not going in the properties table.
    foreach ($instances_for_field_groups as $key => $other_field) {
      // Temporary field names.
      $temporary_field = array();
      $group_field_name = 'gp_'.$other_field['field_name'];

      // Need to truncate the names because of database field size restrictions,
      // updating fields here to ensure name consistency.
      $group_field_name = substr($group_field_name, 0, 27);

      // Add randomm numbers to ensure the field name is unique within the 32
      // character limit of the field.
      $group_field_name = $group_field_name.rand(0, 99999);

      // Build the field group.
      _additional_fields_field_group_info($bundle_name, $other_field['label'], $other_field['field_name'], $other_field['field_name']);

      // Update arrays.
      array_push($temporary_field, $group_field_name, $other_field['field_name']);
      $region_right = array_merge($region_right, $temporary_field);
      $all_fields += [ $group_field_name => 'right', ];
    }

    //Properties table fields.
    if(!empty($properties)){
      _publication_prop_field_group_info($bundle_name, $properties);
      array_unshift($properties, 'group_prop_tripalpane', 'group_prop_table');
      $region_right = array_merge($region_right, $properties);
      $all_fields+= [ 'group_prop_tripalpane' => 'right', 'group_prop_table' => 'right' ];
    }

    // Add blocks to $region_left and build the toc field that is placed within.
    _ds_fields_info_write($bundle_name);
    $region_left += [ 'toc' ];
    $all_fields += [ 'toc' => 'left' ];
    // Build the ds layout.
    $record = new stdClass;
    $record->id ='TripalEntity|' . $bundle_name . '|default';
    $record->entity_type = 'TripalEntity';
    $record->bundle = $bundle_name;
    $record->view_mode = 'default';
    $record->layout = 'tripal_ds_feature';
    $settings = array(
      'regions' => array(
        'left' =>
          $region_left,
        'right' =>
          $region_right,
      ),
      'fields' =>
        $all_fields,
      'classes' => array(),
      'wrappers' => array(
        'left' => 'div',
        'right' => 'div',
      ),
      'layout_wrapper' => 'div',
      'layout_attributes' => '',
      'layout_attributes_merge' => 1,
      'layout_link_attribute' => '',
      'layout_link_custom' => '',
      'layout_disable_css' => 0,
    );
    $record->settings = $settings;
    drupal_write_record('ds_layout_settings', $record);
  }
  catch (Exception $e) {
    watchdog_exception('tripal_ds', $e);
    return FALSE;
  }
  return TRUE;
}

/**
 * Implements hook_ds_fields_info().
 */
function tripal_ds_ds_fields_info($entity_type) {
  $fields = array();

  $fields['toc'] = array(
  'title' => t('Table of Contents'),
  'field_type' => DS_FIELD_TYPE_FUNCTION,
  'function' => 'tripal_ds_toc_block',
  );
  return array('TripalEntity' => $fields);
}

/**
 *
 * @param $entity_type
 * @return
 */
function tripal_ds_toc_block($entity_type) {
  $bundle_name = $entity_type['bundle'];
  $toc = views_embed_view('tripal_content_type_toc', 'block', $bundle_name);
  return $toc;
}

function _ds_fields_info_write($bundle_name) {
  $fields = new stdClass;
  $fields->id ='TripalEntity|' . $bundle_name . '|default';
  $fields->entity_type = 'TripalEntity';
  $fields->bundle = $bundle_name;
  $fields->view_mode = 'default';
  $fields->settings = array(
    'toc' => array(
      'weight' => 0,
      'label' => 'hidden',
      'format' => 'default',
    ),
  );
  drupal_write_record('ds_field_settings', $fields);
}