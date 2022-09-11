<?php

/**
 * @file Nodeaccess admin forms.
 */

/**
 * Menu callback. Draws the admin page.
 *
 * @param $form
 * @param $form_state
 *
 * @return array $form
 */
function nodeaccess_admin_form($form, $form_state) {
  // Load in our configurations
  $config = config('nodeaccess.settings');

  /**
   * In the event new roles get added, we need to add them to our role map.
   * I am not crazy about this, but I can't find a hook that fires when a
   * new role is created.
   */
  $roles = user_roles(FALSE, NULL, TRUE);
  $i = 0;
  $roles_gids = array();
  foreach ($roles as $role_id => $role) {
    $roles_gids[$i] = new stdClass;
    $roles_gids[$i]->name = $role->name;
    $roles_gids[$i]->weight = $role->weight;
    $i++;
  }
  $config->set('role_map', $roles_gids);
  $config->save();

  // Set defaults from configuration management
  $show = $config->get('types');
  $roles = $config->get('role_map');

  $allowed_roles = $config->get('allowed_roles');
  $allowed_grants = $config->get('allowed_grants');

  $form['priority'] = array(
    '#type' => 'checkbox',
    '#title' => t('Give node grants priority'),
    '#default_value' => $config->get('priority'),
    '#description' => t('If you are only using this access control module,
      you can safely ignore this. If you are using multiple access control
      modules, and you want the grants given on individual nodes to override
      any grants given by other modules, you should check this box.'),
  );

  // Select whether to preserve hidden grants.
  $form['preserve'] = array(
    '#type' => 'checkbox',
    '#title' => t('Preserve hidden grants'),
    '#default_value' => $config->get('preserve'),
    '#description' => '<small>' . t('If you check this box, any hidden grants
      are preserved when you save grants. Otherwise all grants users are not
      allowed to view or edit are revoked on save.') . '</small>',
  );

  // Select permissions you want to allow users to view and edit.
  $form['grant'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Allowed Grants'),
    '#tree' => TRUE,
    '#description' => '<small>' . t('The selected grants will be listed on
      individual node grants. If you wish for certain grants to be hidden from
      users on the node grants tab, make sure they are not selected here.') .
      '</small>',
  );
  $form['grant']['view'] = array(
    '#type' => 'checkbox',
    '#title' => t('View'),
    '#default_value' => $allowed_grants['view'],
  );
  $form['grant']['edit'] = array(
    '#type' => 'checkbox',
    '#title' => t('Edit'),
    '#default_value' => $allowed_grants['edit'],
  );
  $form['grant']['delete'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete'),
    '#default_value' => $allowed_grants['delete'],
  );

  // Select roles the permissions of which you want to allow users to
  // view and edit.
  $form['role'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Allowed Roles'),
    '#tree' => TRUE,
    '#theme' => 'nodeaccess_admin_form_roles',
    '#description' => t('The selected roles will be listed on individual node
      grants. If you wish for certain roles to be hidden from users on the node
      grants tab, make sure they are not selected here.'),
  );

  foreach ($roles as $id => $role) {
    if (!$role->weight) {
      $role->weight = 0;
    }
    $form['role'][$id]['name'] = array(
      '#type' => 'hidden',
      '#value' => $role->name,
    );
    $form['role'][$id]['allow'] = array(
      '#type' => 'checkbox',
      '#title' => check_plain($role->name),
      '#default_value' => isset($allowed_roles[$id]) ? $allowed_roles[$id] : 0,
    );
  }

  // Grant tab to node types.
  $form['nodeaccess']['tabs'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Content Type Grant Tab Access'),
    '#tree' => TRUE,
    '#description' => t('Show grant tab for the following node types.'),
  );

  $options = array();
  $node_types = node_type_get_types();
  foreach ($node_types as $type => $bundle) {
    $options[$type] = array(
      'content_type' => check_plain($bundle->name),
    );
  }
  $form['nodeaccess']['tabs']['show'] = array(
    '#type' => 'tableselect',
    '#header' => array(
      'content_type' => t('Content type'),
    ),
    '#options' => $options,
    '#default_value' => $show,
    '#empty' => t('No content types to add a grant tab.'),
  );

  // Generate fieldsets for each node type.
  foreach ($node_types as $type => $bundle) {
    $form['nodeaccess'][$type] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => check_plain($bundle->name),
      '#tree' => TRUE,
      '#theme' => 'nodeaccess_admin_form_types',
    );

    // Set default author permissions for node type.
    $author_prefs = $config->get('authors');
    $form['nodeaccess'][$type]['author']['grant_view'] = array(
      '#type' => 'checkbox',
      '#default_value' => $author_prefs[$type]['grant_view'],
    );
    $form['nodeaccess'][$type]['author']['grant_update'] = array(
      '#type' => 'checkbox',
      '#default_value' => $author_prefs[$type]['grant_update'],
    );
    $form['nodeaccess'][$type]['author']['grant_delete'] = array(
      '#type' => 'checkbox',
      '#default_value' => $author_prefs[$type]['grant_delete'],
    );

    // Set default role permissions for node type.
    $opts = $config->get('nodeaccess_' . $type);
    $user_roles = user_roles();
    foreach ($user_roles as $id => $role) {
      $form['nodeaccess'][$type]['roles'][$id]['name'] = array('#markup' => $role);
      $form['nodeaccess'][$type]['roles'][$id]['grant_view'] = array(
        '#type' => 'checkbox',
        '#default_value' => isset($opts[$id]['grant_view']) ? $opts[$id]['grant_view'] : 0,
      );
      $form['nodeaccess'][$type]['roles'][$id]['grant_update'] = array(
        '#type' => 'checkbox',
        '#default_value' => isset($opts[$id]['grant_update']) ? $opts[$id]['grant_update'] : 0,
      );
      $form['nodeaccess'][$type]['roles'][$id]['grant_delete'] = array(
        '#type' => 'checkbox',
        '#default_value' => isset($opts[$id]['grant_delete']) ? $opts[$id]['grant_delete'] : 0,
      );
    }

    // Set the default permissions if userreference exists and is enabled on
    // the content type.
    if (module_exists('user_reference')) {
      $bundle = field_extract_bundle('node', $bundle);
      $fields = field_info_instances('node', $bundle);
      $user_reference_perms = $config->get('nodeaccess_' . $type . '_user_reference');

      $field_types = field_info_field_types();

      foreach ($fields as $field) {
        $field = field_info_field($field['field_name']);
        if ($field['type'] == 'user_reference') {
          $enabled = isset($user_reference_perms[$field['field_name']]['enabled']) ?
            $user_reference_perms[$field['field_name']]['enabled'] : 0;
          $view = isset($user_reference_perms[$field['field_name']]['grant_view']) ?
            $user_reference_perms[$field['field_name']]['grant_view'] : 0;
          $update = isset($user_reference_perms[$field['field_name']]['grant_update']) ?
            $user_reference_perms[$field['field_name']]['grant_update'] : 0;
          $delete = isset($user_reference_perms[$field['field_name']]['grant_delete']) ?
            $user_reference_perms[$field['field_name']]['grant_delete'] : 0;

          $form['nodeaccess'][$type]['user_reference'][$field['field_name']]['name'] = array(
            '#value' => t($field_types[$field['type']]['label']),
          );
          $form['nodeaccess'][$type]['user_reference'][$field['field_name']]['enabled'] = array(
            '#type' => 'checkbox',
            '#default_value' => $enabled,
          );
          $form['nodeaccess'][$type]['user_reference'][$field['field_name']]['grant_view'] = array(
            '#type' => 'checkbox',
            '#default_value' => $view,
          );
          $form['nodeaccess'][$type]['user_reference'][$field['field_name']]['grant_update'] = array(
            '#type' => 'checkbox',
            '#default_value' => $update,
          );
          $form['nodeaccess'][$type]['user_reference'][$field['field_name']]['grant_delete'] = array(
            '#type' => 'checkbox',
            '#default_value' => $delete,
          );
        }
      }
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Grants'),
  );
  return $form;
}

/**
 * Submit function for nodeaccess_admin_form.
 *
 * @param $form
 * @param $form_state
 */
function nodeaccess_admin_form_submit($form, $form_state) {
  // Load in our configurations
  $config = config('nodeaccess.settings');

  $form_values = $form_state['values'];
  $allowed_grants = array();
  // Save priority.
  $config->set('priority', $form_values['priority']);
  // Save preserve.
  $config->set('preserve', $form_values['preserve']);
  // Save allowed grants.
  foreach ($form_values['grant'] as $id => $val) {
    $allowed_grants[$id] = $val;
  }
  $config->set('allowed_grants', $allowed_grants);

  // Iterate through the allowed roles and re-save into our 
  // configuration. No aliases. Just names and weights.
  $allowed_roles = array();
  foreach ($form_values['role'] as $id => $val) {
    $allowed_roles[$id] = $val['allow'];
  }
  $config->set('allowed_roles', $allowed_roles);

  // Save author and role permissions for each node type.
  $author_prefs = array();
  $node_types = node_type_get_types();
  foreach ($node_types as $type => $name) {
    $grants = array();
    foreach ($form_values[$type]['roles'] as $role => $val) {
      $gid = nodeaccess_get_role_id_from_machine($role);
      $grants[$role] = array(
        'gid' => $gid,
        'realm' => 'nodeaccess_rid',
        'grant_view' => $val['grant_view'],
        'grant_update' => $val['grant_update'],
        'grant_delete' => $val['grant_delete'],
      );
    }

    $config->set('nodeaccess_' . $type, $grants);
    $author_prefs[$type] = $form_values[$type]['author'];
    // Also save userreference default permissions if enabled.
    if (module_exists('user_reference') && isset($form_values[$type]['user_reference'])) {
      $user_reference_grants = array();
      foreach ($form_values[$type]['user_reference'] as $user_reference_field => $val) {
        $user_reference_grants[$user_reference_field] = array(
          'gid' => 'nodeaccess_uid',
          'enabled' => $val['enabled'],
          'grant_view' => $val['grant_view'],
          'grant_update' => $val['grant_update'],
          'grant_delete' => $val['grant_delete'],
        );
      }
      $config->set('nodeaccess_' . $type . '_user_reference', $user_reference_grants);
    }
  }
  $config->set('authors', $author_prefs);

  // Save allowed node type grant tab.
  $allowed_types = array();
  foreach ($form_values['tabs']['show'] as $type => $value) {
    $allowed_types[$type] = (bool) $value;
  }
  $config->set('types', $allowed_types);
  $config->save();
  nodeaccess_set_type_grants($allowed_types);
  backdrop_set_message(t('Grants saved.'));
}

/**
 * Theme function to render admin form.
 *
 * @param $variables
 *
 * @return string
 */
function theme_nodeaccess_admin_form_types($variables) {
  $form = $variables['form'];
  $output = backdrop_render($form['show']);
  $roles = element_children($form['roles']);
  $header = array(t('Role'), t('View'), t('Edit'), t('Delete'));
  $rows = array();
  foreach ($roles as $role) {
    $row = array();
    $row[] = backdrop_render($form['roles'][$role]['name']);
    $row[] = backdrop_render($form['roles'][$role]['grant_view']);
    $row[] = backdrop_render($form['roles'][$role]['grant_update']);
    $row[] = backdrop_render($form['roles'][$role]['grant_delete']);
    $rows[] = $row;
  }
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $header = array(t('Author Settings'), t('View'), t('Edit'), t('Delete'));
  $row = array();
  $row[] = t('Node author');
  $row[] = backdrop_render($form['author']['grant_view']);
  $row[] = backdrop_render($form['author']['grant_update']);
  $row[] = backdrop_render($form['author']['grant_delete']);

  $output .= theme('table', array('header' => $header, 'rows' => array($row)));

  $output .= '<small>' . t('The settings selected for the node author will
    define what permissions the node author has. This cannot be changed on
    individual node grants.') . '</small>';

  if (module_exists('user_reference') && isset($form['user_reference'])) {
    $user_reference_fields = element_children($form['user_reference']);
    $header = array(t('User Reference Field'),
      t('Enable this field'),
      t('View'),
      t('Edit'),
      t('Delete'),
    );
    $rows = array();
    foreach ($user_reference_fields as $user_reference_field) {
      $row = array();

      $row[] = $form['user_reference'][$user_reference_field]['name']['#value'];
      $row[] = backdrop_render($form['user_reference'][$user_reference_field]['enabled']);
      $row[] = backdrop_render($form['user_reference'][$user_reference_field]['grant_view']);
      $row[] = backdrop_render($form['user_reference'][$user_reference_field]['grant_update']);
      $row[] = backdrop_render_children($form['user_reference'][$user_reference_field]['grant_delete']);
      $rows[] = $row;
    }
    $output .= theme('table', array(
      'header' => $header,
      'rows' => $rows,
    ));
    $output .= '<small>' . t('If enabled, the value of the user reference field
      will be granted the associated permissions. If a user changes the value of
      the user reference field on a node, the associated user will be modified
      in the node-specific access table.') . '</small>';
  }
  return $output;
}
