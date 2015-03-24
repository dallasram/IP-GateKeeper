<?php

/**
 * @file
 * Page callback file for the ip ranges module.
 */

/**
 * Menu callback. Displays banned IP ranges.
 */
function ip_gatekeeper_testpage() {
  drupal_set_message('<a href="/admin/config/people/ip_gatekeeper-ip-ranges/settings">Click here for ip_gatekeeper settings.</a>');
  drupal_set_message('<strong>Note that your own IP-Address is currently <i>'.ip_address().'</i>. Be carefull not to lock yourself out!<br /></strong>');

  // Check to see if any existing restricted aliases no longer exist
  $ips = ip_gatekeeper_get_ip_list();

  // Perform a basic check to determine if restricted paths are still valid
  // @TODO: Check/Test this logic.
  foreach($ips as $record) {
      // Determine if the entered path is a path Drupal recognizes
      if (!drupal_valid_path($record->path)) {
        // Not a system URL.
        if (!drupal_lookup_path('source', $record->path)) {
          // Not a path alias.
          // $path does not exits
          drupal_set_message($record->path . " may not be a valid path, please check!");
        }
        else if (!drupal_lookup_path('source', $record->source) && !drupal_valid_path($record->source)) {
            drupal_set_message($record->path . " Possibly an abandonded alias!");
        }
      }
  }

  $build = array();

  $build['ip_gatekeeper_form'] = drupal_get_form('ip_gatekeeper_whitelist_page_form');

  $rows = array();
  $header = array(t('Path'), t('Source'), t('Allowed IP'), t('Operations'));
  $ip_list = ip_gatekeeper_get_ip_list();
  foreach ($ip_list as $ip) {
    $rows[] = array(
      $ip->path,
      $ip->source,
      str_replace('-', ' - ',$ip->ip),
      l(t('delete'), "admin/config/people/ip_gatekeeper-ip-ranges/delete/$ip->bid/$ip->ip"),
    );
  }

  $build['ip_gatekeeper_restricted_pages_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  return $build;
}

/**
 * Menu callback. Displays the IP Gatekeeper Settings Page.
 */
function ip_gatekeeper_settings_form() {
	$form = array();

	$options = array(0 => t('Standard Drupal Access Denied Page'), 1 => t('Boot Mode: Resource Friendly, only the customizable error message below will be shown.'));

    $form['ip_gatekeeper_bootmode'] = array(
     '#type' => 'radios',
     '#title' => t('Boot Mode'),
     '#default_value' => variable_get('ip_gatekeeper_bootmode', 0),
     '#options' => $options,
     '#description' => t('To save server resources, it is recommended to use Boot Mode.'),
   );

   $form['ip_gatekeeper_bootmode_text'] = array(
    '#title' => t('Boot Mode: Enter the text you wish to appear on unauthorized pages.'),
    '#type' => 'textfield',
    '#size' => 48,
    '#required' => TRUE,
    '#maxlength' => 255,
    '#default_value' => variable_get('ip_gatekeeper_bootmode_text', t("Sorry, something went wrong.  That's all we know.")),
    '#description' => t('This text only applies when Boot Mode is enabled.'),
  );

   return system_settings_form($form);
}

/**
 * The form for creating a restricted path entry.
 */
function ip_gatekeeper_whitelist_page_form($form, $form_state) {
  $form['ip_path'] = array(
    '#title' => t('Enter the Path you wish to restrict.'),
    '#type' => 'textfield',
    '#size' => 48,
    '#required' => TRUE,
    '#maxlength' => 255,
    '#description' => t('Wildcards in path are not supported at this time. Please do not include the preceeding or trailing slash. E.g. node/11'),
  );

  $form['ip_start'] = array(
    '#title' => t('Allowed IP range start / Single IP-address'),
    '#type' => 'textfield',
    '#size' => 48,
    '#required' => TRUE,
    '#maxlength' => 15,
    '#description' => t('Enter IP-address (100.100.100.100). If range end is specified, it will be used as start of the range, otherwise as a single IP-address.'),
  );
  $form['ip_end'] = array(
    '#title' => t('Allowed IP range end (optional)'),
    '#type' => 'textfield',
    '#size' => 48,
    '#required' => FALSE,
    '#maxlength' => 15,
    '#description' => t('If entered, the banned ip will be treated as a range.'),
  );

  $form['type'] = array(
    '#type' => 'hidden',
    '#value' => 'whitelist',
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );
  $form['#validate'][] = 'ip_gatekeeper_form_validate';
  $form['#submit'][] = 'ip_gatekeeper_form_submit';

  return $form;
}

/**
 * Form validation handler for ip_gatekeeper_form().
 */
function ip_gatekeeper_form_validate($form, &$form_state) {
  // Determine if the entered path is a path Drupal recognizes
  if (!drupal_valid_path(trim($form_state['values']['ip_path']))) {
      // Not a system URL.
      if (!drupal_lookup_path('source', trim($form_state['values']['ip_path']))) {
        // Not a path alias.
        // $path does not exits
        form_set_error('ip_path', t("The path you have entered is not recognized."));
      }
    }
  $ip_start = trim($form_state['values']['ip_start']);
  $ip_end = isset($form_state['values']['ip_end']) ? trim($form_state['values']['ip_end']) : FALSE;
  if (filter_var($ip_start, FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE) == FALSE) {
    form_set_error('ip_start', t("IP(/range start) is not a valid IP address."));
  }
  elseif ($ip_end && filter_var($ip_end, FILTER_VALIDATE_IP, FILTER_FLAG_NO_RES_RANGE) == FALSE) {
    form_set_error('ip_end', t("IP range end is not a valid IP address."));
  }
  elseif ($ip_start == ip_address() || ($ip_end && ip_gatekeeper_check_range($ip_start . '-' . $ip_end, ip_address()))) {
  // This fires if users own IP is in the selected range. Currently we wont do anything in that case.
 }
}

/**
 * Form submission handler for ip_gatekeeper_form().
 */
function ip_gatekeeper_form_submit($form, &$form_state) {
  $ip_path = trim($form_state['values']['ip_path']);
  $ip_start = trim($form_state['values']['ip_start']);
  $ip_start = preg_replace('/\s+/', '', $ip_start);
  $ip = $ip_start;

  if ($form_state['values']['ip_end']) {
    $ip_end = trim($form_state['values']['ip_end']);
    $ip_end = preg_replace('/\s+/', '', $ip_end);
    $ip.='-'.$ip_end;
  }

  $type = $form_state['values']['type'];

  // Determine all the paths that can be associated with the entered path (i.e. system path and all aliased paths)
  $saved_paths = array();

  // Get the system path
  $saved_paths['system'] = drupal_get_normal_path($ip_path);
  //dpm('Normal path: ' . drupal_get_normal_path($ip_path) );

  // Now save each alias to this system path
  $aliases = db_query('
              SELECT alias
              FROM {url_alias}
              WHERE source = :source',
              array(':source' => $saved_paths['system']))->fetchAll();
  if (isset($aliases) && !empty($aliases)) {
      foreach($aliases as $index => $alias) {
          $saved_paths['alias'.$index] = $alias->alias;
      }
  }

  // Save each known path alias
  foreach($saved_paths as $ip_path) {
    db_insert('ip_gatekeeper')
        ->fields(array('source' => $saved_paths['system'], 'path' => $ip_path, 'ip' => $ip, 'type' => $type))
        ->execute();
    if ($ip_path == $saved_paths['system']) {
      drupal_set_message(t('Source path of !source_path has been saved', array('!source_path' => $ip_path)));
    }
    else {
      drupal_set_message(t('Alias of !source_path has been found and saved (!alias)',
                            array(
                                  '!source_path' => $saved_paths['system'],
                                  '!alias' => $ip_path
                                  )
                          );
    }
  }

  $form_state['redirect'] = 'admin/config/people/ip_gatekeeper-ip-ranges';
  return;
}

/**
 * IP deletion confirm page.
 *
 * @see ip_gatekeeper_delete_submit()
 */
function ip_gatekeeper_delete($form, &$form_state, $bid, $ip) {
  $form['ip_range'] = array(
    '#type' => 'value',
    '#value' => $bid,
  );

  return confirm_form($form, t('Are you sure you want allow _rpath_ to be accessible by ip/range %ip?', array( '%ip' => str_replace('-',' - ',$ip))), 'admin/config/people/ip_gatekeeper-ip-ranges', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Processes ip_gatekeeper_delete form submissions.
 */
function ip_gatekeeper_delete_submit($form, &$form_state) {
  $ip = $form_state['build_info']['args']['1'];
  $bid = $form_state['values']['ip_range'];
  db_delete('ip_gatekeeper')
    ->condition('bid', $bid)
    ->execute();
  watchdog('user', 'Deleted %ip from banned ip-ranges', array('%ip' => $ip));
  drupal_set_message(t('The IP address/range %ip was deleted.', array('%ip' => str_replace('-',' - ',$ip))));
  $form_state['redirect'] = 'admin/config/people/ip_gatekeeper-ip-ranges';
}
