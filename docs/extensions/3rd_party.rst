Third-party Integration
=======================

The following modules provide integration with external third-party tools. For example, they may allow you to easily embed the tool in Drupal/Tripal pages and/or expose data from the tool on your Tripal site.

Tripal Blast
------------

This module provides a basic interface to allow your users to utilize your server's NCBI BLAST+. There is a simple interface allowing users to paste or upload a query sequence and then select from available databases and a tabular results listing with alignment information and multiple download formats (HTML, TSV, GFF3, XML) available.

`Documentation <https://github.com/tripal/tripal_blast/blob/7.x-1.x/README.md>`__
`Repository <https://github.com/tripal/tripal_blast>`__

Tripal Galaxy
-------------

This module is for integration of Tripal and the Galaxy Project. It facilitates the creation of forms on your Tripal site that submit jobs to a galaxy instance.

`Documentation <https://tripal-galaxy.readthedocs.io/en/latest/?badge=latest>`__
`Repository <https://github.com/tripal/tripal_galaxy>`__

Tripal JBrowse
--------------

This module provides integration between Tripal sites and pre-existing GMOD JBrowse instances. It allows you to create pages on your Tripal site with an embedded JBrowse instance by filling out a simple form.

`Documentation <https://github.com/tripal/tripal_jbrowse/blob/7.x-2.1.x/README.md>`__
`Repository <https://github.com/tripal/tripal_jbrowse>`__

VCF Filter
-----------

This modules provides a form interface so users can custom filter existing VCF files and export in a variety of formats. The form simply provides an interface to VCFtools and uses the Tripal Download API to provide the filtered file to the user.

`Documentation <https://github.com/UofS-Pulse-Binfo/vcf_filter/blob/master/README.md>`__
`Repository <https://github.com/UofS-Pulse-Binfo/vcf_filter>`__
