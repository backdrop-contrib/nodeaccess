NodeAccess Module
=================

This module allows you to manage permissions for nodes/content by role
and user. In other words, it implements per node access control for users and
groups. With this module, you can restrict access to any individual node 
without having to use taxonomies. You can assign permission to view, edit or
delete each individual piece of content by user or role. Once enabled, a 
'grant' tab will appear on edit pages for each piece of content. You can click 
this and assign permissions for that node/piece of content.

Installation
------------

Put the module in your Backdrop modules directory and enable it in
admin/modules.

I would strongly recommend backing up your database before installing this
module.

IMPORTANT!

Once you check the enable box and submit the page, no nodes will be accessible
to anyone other than the admin user. You just set permissions on the nodeaccess
settings page (admin/settings/nodeaccess) to enable access to your site.

The settings page has a section for roles, and then a section for every node 
type you have on your site.

Documentation
------------

Additional documentation is located in [the Wiki](https://github.com/backdrop-contrib/nodeaccess/wiki/Documentation).

Issues
------

Bugs and feature requests should be reported in [the Issue Queue](https://github.com/backdrop-contrib/nodeaccess/issues).

Current Maintainers
-------------------

- [Michael R. Bagnall](https://github.com/ElusiveMind).

Credits
-------

- Ported to Backdrop CMS by [Michael R. Bagnall](https://github.com/ElusiveMind).
- Originally written for Drupal by Chad Cunningham.

License
-------

This project is GPL v2 software.
See the LICENSE.txt file in this directory for complete text.
