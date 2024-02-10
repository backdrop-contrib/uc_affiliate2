<?php
/**
 * @file
 * Admin functions for uc_affiliate2 module.
 *
 */

/**
 * The ensure the date and interval used in the reports remain consistant
 * across all pages we define them here.
 */
define('UC_AFFILIATE2_REPORT_DATE_FORMAT', 'M-Y');
define('UC_AFFILIATE2_REPORT_DATE_INTERVAL', '1 month');

/**
 * Define the affiliate users listing.
 */
function uc_affiliate2_admin_manage($user_id = NULL) {
  if (!is_null($user_id) && is_numeric($user_id)) {
    return uc_affiliate2_admin_users($user_id);
  }

  $clicks = array();
  $users = array();
  $orders = array();

  $result = db_query("SELECT aid, COUNT(click_id) as clicks FROM {uc_affiliate2_clicks} GROUP BY aid");
  foreach ($result as $record) {
    $clicks[$record->aid] = $record->clicks;
  }

  $result = db_query("SELECT aid, COUNT(aid) as users FROM {uc_affiliate2_users} GROUP BY aid");
  foreach ($result as $record) {
    $users[$record->aid] = $record->users;
  }

  $result = db_query("SELECT aid, COUNT(aid) as orders FROM {uc_affiliate2_orders} GROUP BY aid");
  foreach ($result as $record) {
    $orders[$record->aid] = $record->orders;
  }

  // setup the table
  $header = array(
    array(
      'data' => t('Username'),
      'field' => 'u.name',
    ),
    array(
      'data' => t('Status'),
      'field' => 'u.status',
    ),
    array(
      'data' => t('Member for'),
      'field' => 'u.created',
      'sort' => 'desc',
    ),
    array(
      'data' => t('Last access'),
      'field' => 'u.access',
    ),
    array('data' => t('Unique Clicks')),
    array('data' => t('Users')),
    array('data' => t('Orders')),
  );

  $affiliate_roles = array_keys(user_roles(FALSE, 'act as affiliate')); 
  $query = db_select('users','u')->extend('PagerDefault')->extend('TableSort')->orderByHeader($header)->limit(30);
  $query->innerJoin('users_roles','ur','u.uid = ur.uid');
  $query->fields('u',array('uid','name','status','created','access'));
  $query->condition('ur.role',$affiliate_roles,'IN');

  $result = $query->execute();
  $status = array(t('blocked'), t('active'));
  $rows=array();
  foreach ($result as $account) {
    $rows[] = array(
      l($account->name, 'admin/store/settings/affiliate/' . $account->uid),
      $status[$account->status],
      format_interval(REQUEST_TIME - $account->created),
      $account->access ? t('@time ago', array('@time' => format_interval(REQUEST_TIME - $account->access))) : t('never'),
      !empty($clicks[$account->uid]) ? $clicks[$account->uid] : '0',
      !empty($users[$account->uid]) ? $users[$account->uid] : '0',
      !empty($orders[$account->uid]) ? $orders[$account->uid] : '0',
    );
  }

  $output = "<p>" . t("Click on an affiliate user to view their monthly sales and commissions.");
  $output .= theme('table',array('header'=> $header,'rows'=> $rows));
  $output .= theme('pager', array('tags' => NULL, 'element' => 0));

  return $output;
}

/**
 * Display user's sales and commissions.
 */
function uc_affiliate2_admin_users($uid) {
  // used to determine whether to show the paid form.
  $month_timestamp = arg(5);

  $account = user_load($uid);

  // setup the table
  $header = array(
    array(
      'data' => t('Month'),
      'field' => 'date',
    ),
    array(
      'data' => t('Sales'),
      'field' => 'sales',
    ),
    array(
      'data' => t('Commissions'),
      'field' => 'commissions',
    ),
    array(
      'data' => t('Paid (click to edit)'),
      'field' => 'paid',
    ),
  );

  // define the query
  $query=db_select('uc_affiliate2_commission','ac');
  $query->leftJoin('uc_orders','uo','ac.order_id = uo.order_id');
  $query->condition('ac.aid',$uid,'=');
  $query->addField('ac','commission');
  $query->fields('uo',array('order_total','created'));
  $result = $query->execute();

  $total_sales = array();
  $total_commissions = array();
  $amounts = _uc_affiliate2_get_amounts('aid', $uid, 'date');

  foreach ($result as $rec) {
    $mondate = date(UC_AFFILIATE2_REPORT_DATE_FORMAT, $rec->created);
    $total_sales[$mondate] = isset($total_sales[$mondate]) ? $total_sales[$mondate] + $rec->order_total : $rec->order_total;
    $total_commissions[$mondate] = isset($total_commissions[$mondate]) ? $total_commissions[$mondate] + $rec->commission : $rec->commission;
  }

  $rows=array();
  foreach ($total_sales as $date => $sales) {
    $timestamp = strtotime($date);
    $amount = isset($amounts[$timestamp]) ? $amounts[$timestamp] : 0;

    // format paid field as link or form
    if ($month_timestamp && $date == $month_timestamp) {
      $amount = uc_currency_format($amount, FALSE);
      $form = backdrop_get_form('uc_affiliate2_admin_users_paid_form', $amount, $uid, strtotime($date));
      $paid = backdrop_render($form);
    }
    else {
      $amount = uc_currency_format($amount);
      $paid = l($amount, implode('/', array(arg(0), arg(1), arg(2), arg(3),arg(4), $date)));
    }

    // generate record
    $rows[] = array(
      str_replace('-', ' ', $date),
      uc_currency_format($sales),
      uc_currency_format($total_commissions[$date]),
      $paid,
    );
  }

  $output = "<p>" . t("Displaying affiliate sales and commissions for ") . theme('username', array('account' => $account)) . ".";

  $output .= theme('table',array('header'=>$header,'rows'=>$rows));;

  return $output;
}

/**
 * Form builder for amount paid.
 */
function uc_affiliate2_admin_users_paid_form($form, $form_state, $amount, $uid, $date) {
  $form['amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount paid'),
    '#default_value' => $amount,
    '#size' => 8,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $uid,
  );
  $form['date'] = array(
    '#type' => 'value',
    '#value' => $date,
  );

  $form_state['#redirect'] = 'admin/store/affiliate/reports/' . date(UC_AFFILIATE2_REPORT_DATE_FORMAT, $date);

  return $form;
}

/**
 * Validation callback for amount paid form.
 */
function uc_affiliate2_admin_users_paid_form_validate($form, &$form_state) {
  $amount = $form_state['values']['amount'];

  if (!is_numeric($amount) || $amount < 0) {
    form_set_error('amount', t('Please specify an amount paid to this affiliate.'));
  }
}

/**
 * Submit handler for amount paid form.
 */
function uc_affiliate2_admin_users_paid_form_submit($form, &$form_state) {
  $sign = config_get('uc_store.settings', 'uc_currency_sign');
  $amount = $form_state['values']['amount'];
  $uid = $form_state['values']['uid'];
  $date = $form_state['values']['date'];

  $rec = db_query('SELECT amount FROM {uc_affiliate2_paid} WHERE aid = :aid AND date = :date', array(':aid' => $uid, ':date' => $date))->fetchField();
  if ($rec === FALSE) {
    $id = db_insert('uc_affiliate2_paid')
      ->fields(array(
        'aid' => $uid,
        'date' => $date,
        'amount' => $amount,
      ))
      ->execute();
  }
  else {
    db_update('uc_affiliate2_paid')
      ->fields(array(
        'amount' => $amount,
      ))
    ->condition('aid', $uid)
    ->condition('date', $date)
    ->execute();
  }

  $account = user_load($uid);
  backdrop_set_message(t('Registered payment of @amount for !user during @date.', array('@amount' => uc_currency_format($amount), '!user' => theme('username', array('account' => $account)), '@date' => format_date($date, 'custom', 'M Y'))));

  return 'admin/store/affiliate/' . $uid;
}

/**
 * Define the banner/link settings form.
 */
function uc_affiliate2_banner_settings($form, &$form_state) {
  $form['#config'] = 'uc_affiliate2.settings';
  $form['affiliate_banner_template'] = array(
    '#type' => 'textarea',
    '#title' => t('Affiliate Banner Templates'),
    '#description' => t('The IMG tag(s) around which the url of the affiliate is wrapped around. Insert as many as you want.'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_banner_template'),
    '#wysiwyg' => FALSE,
  );
  $form['affiliate_text_link_template'] = array(
    '#type' => 'textfield',
    '#title' => t('Affiliate Text Link Template'),
    '#description' => t('The text around which the url of the affiliate is wrapped around. HTML is allowed.'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_text_link_template'),
  );
  $form['affiliate_product_html_template'] = array(
    '#type' => 'textarea',
    '#title' => t('Affiliate Product HTML Template (used for Export)'),
    '#description' => t('When using the html export function in the affiliate panel, this template is used for each product.
                        Allowed placeholders: [LINK], [TITLE], [IMG], [DESCRIPTION], [PRICE].'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_product_html_template'),
    '#wysiwyg' => FALSE,
  );

  return system_settings_form($form);
}

/**
 * Define the commission settings form.
 */
function uc_affiliate2_commission_settings($form, &$form_state) {
  $form['#config'] = 'uc_affiliate2.settings';
  $form['affiliate_hierarchy_depth'] = array(
    '#type' => 'textfield',
    '#title' => t('Affiliate hierarchy depth'),
    '#description' => t('Controls the depth to which affiliates referrals and commissions are tracked'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_hierarchy_depth'),
    '#size' => 3,
  );
  $form['affiliate_commission_structure'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Affiliate Commission Structure'),
    '#description' => t("Sets the default commission structure.  This should be a comma separated list of number with the first value corresponding to the first level in the hierarchy and each successive value corresponding to the next level in the hierarchy. Commission can either be represented as percentages by adding a '%' symbol after the number or as a fixed amount in the stores currency by a number with no symbol. Example: 5%,4%,2%,2%,1%"),
    '#default_value' => implode(',', config_get('uc_affiliate2.settings', 'affiliate_commission_structure')),
    '#size' => 40,
  );

  $roles = user_roles(FALSE, 'act as affiliate');

  foreach ($roles as $role_name => $role) {
    $form['affiliate_settings_role_' . $role_name] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings for @role', array('@role' => $role)),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    $structure_for_role = config_get('uc_affiliate2.settings', 'affiliate_commission_structure_' . $role_name);
    if (empty($structure_for_role)) {
      // Set up the default values.
      $structure_for_role = array(
        '5%',
        '3%',
        '2%',
        '2%',
        '1%'
      );
    }
    $form['affiliate_settings_role_' . $role_name]['affiliate_commission_structure_' . $role_name] = array(
      '#type' => 'textfield',
      '#title' => t('Affiliate Commission Structure for cookied role: @role', array('@role' => $role)),
      '#description' => t("Sets the commission structure for the role, when the user has returned via cookie. This should be a comma separated list of number with the first value corresponding to the first level in the hierarchy and each successive value corresponding to the next level in the hierarchy. Commission can either be represented as percentages by adding a '%' symbol after the number or as a fixed amount in the stores currency by a number with no symbol. Example: 5%,4%,2%,2%,1%"),
      '#default_value' => implode(',', $structure_for_role),
      '#size' => 40,
    );
  }
  return system_settings_form($form);
}

/**
 * Validate function for commission settings form.
 */
function uc_affiliate2_commission_settings_validate($form, &$form_state) {
  $roles = user_roles(FALSE, 'act as affiliate');
  foreach ($roles as $role_name => $role) {
    $form_state['values']['affiliate_commission_structure_' . $role_name] = explode(',', $form_state['values']['affiliate_commission_structure_' . $role_name]);
  }
  $form_state['values']['affiliate_commission_structure'] = explode(',', $form_state['values']['affiliate_commission_structure']);
}

/**
 * Define the affiliate admin settings form.
 */
function uc_affiliate2_admin_settings($form, &$form_state) {
  $form['#config'] = 'uc_affiliate2.settings';

  $form['affiliate_enable_upline'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable the "Upline" page for affiliates.'),
    '#description' => t('This page allows the affiliates to see the affiliates above them (if any) in a multilevel setup.'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_enable_upline'),
  );

  foreach (uc_order_status_list('general') as $status) {
    $statuses[$status['id']] = $status['title'];
  }
  $form['affiliate_commission_order_statuses'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Order status'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_commission_order_statuses'),
    '#description' => t('Where in the order status(es) will the commissions be assigned.'),
    '#options' => $statuses,
  );
  $form['affiliate_invalid_redirect'] = array(
    '#type' => 'textfield',
    '#title' => t('On an invalid affiliate URL, redirect to'),
    '#description' => t('When an invalid username is used as an affiliate, the user will be redirected to this URL.'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_invalid_redirect'),
  );

  $form['affiliate_default_link_type'] = array(
    '#type' => 'radios',
    '#title' => t('Default affiliate link type'),
    '#options' => array(
      'path' => t('Use a special path (!example)', array('!example' => 'http://example.com/<strong>' . check_plain(config_get('uc_affiliate2.settings', 'uc_affiliate2_path')) . '/123</strong>')),
      'query' => t('Use a query argument (!example)', array('!example' => 'http://example.com/<strong>?a=123</strong>')),
    ),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_default_link_type'),
  );

  // Only display this setting if Ubercart Payment is enabled. This prevents a WSOD.
  if (function_exists('_uc_payment_method_list')) {
    foreach (_uc_payment_method_list() as $method) {
      $payment_methods[$method['id']] = $method['name'];
    }

    $form['affiliate_no_commission_payment_methods'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Disable commission for these Payment methods'),
      '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_no_commission_payment_methods'),
      '#description' => t('You can disable commission for certain Payment methods'),
      '#options' => $payment_methods,
    );
  }

  $ip_masks = array(
    'hide' => t("Hide IP addresses from users"),
    'one' => t("Show the last number (*.*.*.123)"),
    'two' => t("Show the last 2 numbers (*.*.123.123)"),
    'first' => t("Show the first number (123.*.*.*)"),
    'full' => t("Show the full IP (primarily for debug purposes)"),
  );
  $form['affiliate_ip_masking'] = array(
    '#type' => 'radios',
    '#title' => t('Mask IP addresses that your affiliates can see'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_ip_masking'),
    '#description' => t('To help affiliates determine who bought what, you can show them the IP address of your customers. This can also be a privacy issue. In the US, do not show more than 2 numbers. Choose wisely. The administrator (user 1) is not affected by this option.'),
    '#options' => $ip_masks,
  );

  $form['affiliate_cookie_tracking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Cookie tracking'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['affiliate_cookie_tracking']['affiliate_set_cookie'] = array(
    '#type' => 'checkbox',
    '#title' => t('Set the affiliate cookie for the visiting user.'),
    '#description' => t('By setting the cookie, the affiliate gets credited even when no affiliate id is present in links visited.'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_set_cookie'),
  );
  $form['affiliate_cookie_tracking']['affiliate_cookie_lifetime'] = array(
    '#type' => 'textfield',
    '#title' => t('Cookie Lifetime'),
    '#description' => t('How long should the cookie be valid. Strtotime syntax.'),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_cookie_lifetime'),
  );
  $form['affiliate_cookie_tracking']['affiliate_subsequent_order_commissions'] = array(
    '#type' => 'radios',
    '#title' => t('Affiliate commissions for future orders placed by users they referred'),
    '#description' => t('If an affiliate cookie is present when a user creates an account, that user is associated with the affiliate. Using this option, you can control whether the affiliate associated with the user gets commissions, or whether any affiliate whose cookies may be present when the user places an order should get the commissions instead.'),
    '#options' => array(
      'none' => t('Only use affiliate cookies that are available at the time of order placement'),
      'enable' => t('Use affiliate cookies at the time of order placement, but if there are none, fall back on the affiliate that first referred the user'),
      'override' => t('Give commissions to the affiliate associated with the user, but if there is none, fall back on any affiliate cookies that may be present'),
      'exclusive' => t('Only give commissions to an affiliate that is associated with the user'),
    ),
    '#default_value' => config_get('uc_affiliate2.settings', 'affiliate_subsequent_order_commissions'),
  );

  return system_settings_form($form);
}

/**
 * Validation handler for affiliate admin settings form.
 */
function uc_affiliate2_admin_settings_validate($form, &$form_state) {
  $url = $form_state['values']['affiliate_invalid_redirect'];

  if (!empty($url) && $url != '<front>' && ($url != check_url($url) || strpos($url, '://') !== FALSE)) {
    form_set_error('affiliate_invalid_redirect', t('Please enter a valid Backdrop path, without the http and domain parts.'));
  }

  if (!strtotime($form_state['values']['affiliate_cookie_lifetime'])) {
    form_set_error('affiliate_cookie_lifetime', t('Invalid strtotime syntax.'));
  }
}

/**
 * Build a monthly report.
 */
function uc_affiliate2_monthly_report($month=null) {
  // The month we are viewing. If this is empty, the current month is assumed.
  if ($month==null) $month= date(UC_AFFILIATE2_REPORT_DATE_FORMAT);
  $date_start = strtotime($month);

  if ($date_start === FALSE || $date_start == -1) { // PHP > 5.1.0 returns FALSE instead of -1
    $month = date(UC_AFFILIATE2_REPORT_DATE_FORMAT);
    $date_start = strtotime($month);
  }
  $date_end = strtotime('+' . UC_AFFILIATE2_REPORT_DATE_INTERVAL, $date_start);

  // Recreate the date to ensure its in the correct format.
  $month = date(UC_AFFILIATE2_REPORT_DATE_FORMAT, $date_start);

  // Used to determine whether to show the paid form
  $uid_external = arg(5);

  // setup the table
  $header = array(
    array(
      'data' => t('Affiliate'),
      'field' => 'aid',
    ),
    array(
      'data' => t('Sales'),
      'field' => 'sales',
    ),
    array(
      'data' => t('Commissions'),
      'field' => 'commissions',
    ),
    array(
      'data' => t('Paid (click to edit)'),
      'field' => 'paid',
    ),
  );

  // define the query
  $query = db_select('uc_affiliate2_commission','ac');
  $query->leftJoin('uc_orders','uo','ac.order_id = uo.order_id');
  $query->condition('uo.created',$date_start,'>=');
  $query->condition('uo.created',$date_end,'<=');
  $query->fields('ac',array('aid','commission'));
  $query->addField('uo','order_total');
  $result=$query->execute();

  $total_sales = array();
  $total_commissions = array();
  $amounts = _uc_affiliate2_get_amounts('date', $date_start, 'aid');

  foreach ($result as $rec) {
    $aid = $rec->aid;
    $total_sales[$aid] = isset($total_sales[$aid]) ? $total_sales[$aid] + $rec->order_total : $rec->order_total;
    $total_commissions[$aid] = isset($total_commissions[$aid]) ? $total_commissions[$aid] + $rec->commission : $rec->commission;
  }

  // lookup the users
  if (count($total_sales)) {
    $uids = explode(',',implode(',',array_keys($total_sales))); //get rid of the array indexes

    $query=db_select('users','u');
    $query->fields('u',array('name','uid'));
    $query->condition('u.uid',$uids,'IN');
    $results=$query->execute();

    $users = array();

    foreach ($results as $user) {
      $users[$user->uid] = $user->name;
    }
  }

  $rows=array();

  foreach ($total_sales as $uid => $sales) {
    $amount = isset($amounts[$uid]) ? $amounts[$uid] : 0;
    $amount = uc_currency_format($amount);

    // format paid field as link or form
    if ($uid_external && is_numeric($uid_external) && $uid == $uid_external) {
      $paid = backdrop_render(backdrop_get_form('uc_affiliate2_admin_users_paid_form', ($amounts[$uid]) ? $amounts[$uid] : 0, $uid, $date_start));
    }
    else {
      $paid = l($amount, implode('/', array(arg(0), arg(1), arg(2), arg(3), $uid, $month)));
    }

    $rows[] = array(
      l($users[$uid], 'user/' . $uid),
      uc_currency_format($sales),
      uc_currency_format($total_commissions[$uid]),
      $paid,
    );
  }

  $output = "<p>" . t("Displaying affiliate sales and commissions for ") . str_replace('-', ' ', $month) . ". ";
  $output .= t("Show ") . l(t('previous month'), implode('/', array(arg(0), arg(1), arg(2), arg(3), arg(4), date(UC_AFFILIATE2_REPORT_DATE_FORMAT, strtotime('-' . UC_AFFILIATE2_REPORT_DATE_INTERVAL, $date_start)))));
  $output .= t(" or ") . l(t('next month'), implode('/', array(arg(0), arg(1), arg(2), arg(3), arg(4), date(UC_AFFILIATE2_REPORT_DATE_FORMAT, $date_end)))) . '.</p>';

  $output .= theme('table', array('header' => $header, 'rows' => $rows));

  return $output;
}

/**
 * Display affiliate commissions.
 */
function uc_affiliate2_order_commissions($order_id) {
  $query = db_select('uc_affiliate2_commission','ac');
  $query->leftJoin('users','u','u.uid=ac.aid');
  $query->condition('ac.order_id',$order_id,'=');
  $query->fields('ac',array('commission_id','aid'));
  $query->addField('u','name');
  $query->fields('ac',array('commission','commission_notes'));
  $result = $query->execute();

  $rows = array();
  foreach ($result as $commission) {
    $rows[] = array(
      l($commission->name, 'user/' . $commission->aid),
      uc_currency_format($commission->commission),
      $commission->commission_notes,
      l(t('edit'), 'admin/store/orders/' . $order_id . '/affiliates/' . $commission->commission_id . '/edit', array('attributes' => array('class' => 'edit'))) . ' | ' .
      l(t('remove'), 'admin/store/orders/' . $order_id . '/affiliates/' . $commission->commission_id . '/remove'),
    );
  }
  return theme('table', array('header' => array('Affiliate', 'Commission', 'Notes', ''), 'rows' => $rows));
}

/**
 * Manually edit a commission on an order.
 */
function uc_affiliate2_order_edit_commissions($form, $form_state, $order_id, $commission_id) {
  $commission = db_query("SELECT commission_id, aid, commission, commission_notes FROM {uc_affiliate2_commission} WHERE commission_id = :commission_id", array(':commission_id' => $commission_id))->fetchObject();

  $form['commission_id'] = array(
    '#type' => 'hidden',
    '#value' => $commission->commission_id,
  );
  $form['order_id'] = array(
    '#type' => 'hidden',
    '#value' => $order_id,
  );
  $form['commission'] = array(
    '#type' => 'textfield',
    '#title' => t('Commission Amount'),
    '#default_value' => $commission->commission,
  );
  $form['commission_notes'] = array(
    '#type' => 'textfield',
    '#title' => t('Commission Note'),
    '#default_value' => $commission->commission_notes,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update commission'),
    '#suffix' => l(t('Cancel'), 'admin/store/orders/' . $order_id . '/affiliates'),
  );
  return $form;
}

/**
 * Submit handler for commission edit form.
 */
function uc_affiliate2_order_edit_commissions_submit($form, &$form_state) {
  $commission = db_query("SELECT * FROM {uc_affiliate2_commission} WHERE commission_id=:commission_id", array(':commission_id' => $form_state['values']['commission_id']))->fetchObject();
  $commission->commission = $form_state['values']['commission'];
  $commission->commission_notes = $form_state['values']['commission_notes'];
  uc_affiliate2_commission_save($commission);
  $form_state['redirect'] = 'admin/store/orders/' . $form_state['values']['order_id'] . '/affiliates';
}

/**
 * Delete a commission from an order.
 */
function uc_affiliate2_order_remove_commissions($form, $form_state, $order_id, $commission_id) {
  $form['commission_id'] = array(
    '#type' => 'hidden',
    '#value' => $commission_id,
  );
  $form['order_id'] = array(
    '#type' => 'hidden',
    '#value' => $order_id,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Remove commission'),
    '#suffix' => l(t('Cancel'), 'admin/store/orders/' . $order_id . '/affiliates'),
  );

  return $form;
}

/**
 * Submit handler for commission elete form.
 */
function uc_affiliate2_order_remove_commissions_submit($form, &$form_state) {
  db_delete('uc_affiliate2_commission')
  ->condition('commission_id', $form_state['values']['commission_id'], '=')
  ->execute();
  $form_state['redirect'] = 'admin/store/orders/' . $form_state['values']['order_id'] . '/affiliates';
}

/**
 * Form to associate attributes with products or classes.
 */
function uc_affiliate2_product_commission_form($form, $form_state, $node) {
  backdrop_add_library('system', 'backdrop.autocomplete');
  $form = array(
    '#theme' => 'uc_affiliate2_product_commission_form',
    '#cache' => TRUE,
  );

  $form['message'] = array(
    '#value' => t("This sets an alternative commission structure for this product. When appling commissions the first commission structure below that matches the affiliate will be used. If the commission structure is set to 0, the product won't be present in the generated links, and no commissions will be given for it. Otherwise this should be a comma separated list of number(s) with the first value corresponding to the first level in the hierarchy and each successive value corresponding to the next level in the hierarchy.  Example: 5%,4%,2%,2%,1%"),
  );

  $num_rules = db_select('uc_affiliate2_product_commission', 'apc')
    ->fields('apc', array('nid'))
    ->countQuery()
    ->execute()->fetchField();
  $num_rules++;

  $form['product_commissions']['#theme'] = 'uc_affiliate2_product_commission_form_current';
  $form['product_commissions']['#tree'] = TRUE;
  // add new rule form
  $rules = _uc_affiliate2_product_commission_rules();

  $form['product_commissions']['new']['rule'] = array(
    '#type' => 'select',
    '#options' =>$rules,
    '#title' => t('Add commissions for...'),
    '#ajax' => array(
      'callback' => 'uc_affiliate2_ajax',
      'wrapper' => 'product-commission-match',
    ),
  );
  $form['product_commissions']['new']['match'] = array(
    '#prefix' => '<div id="product-commission-match">',
    '#suffix' => '</div>',
  );

  if (!empty($form_state['values']['product_commissions']['new']['rule'])) {
    switch ($form_state['values']['product_commissions']['new']['rule']) {
      case 'user':
        $form['product_commissions']['new']['match']['#type'] = 'textfield';
        $form['product_commissions']['new']['match']['#title'] = t('User name');
        $form['product_commissions']['new']['match']['#size'] = 12;
        $form['product_commissions']['new']['match']['#autocomplete_path'] = 'user/autocomplete';
        break;
      case 'role':
        $roles = user_roles(FALSE, 'act as affiliate');
        $form['product_commissions']['new']['match']['#type'] = 'select';
        $form['product_commissions']['new']['match']['#title'] = t('Role');
        $form['product_commissions']['new']['match']['#options'] = $roles;
        $form['product_commissions']['new']['match']['#required'] = TRUE;
        break;
      default:
        $form['product_commissions']['new']['match']['#type'] = 'value';
        $form['product_commissions']['new']['match']['#value'] = 'all';
    }
  }
  else {
    $form['product_commissions']['new']['match']['#markup'] = '';
  }

  $form['product_commissions']['new']['model'] = array(
    '#type' => 'select',
    '#options' => uc_product_get_models($node->nid),
  );
  $form['product_commissions']['new']['commission_structure'] = array(
    '#type' => 'textfield',
    '#size' => 12,
  );
  $form['product_commissions']['new']['weight'] = array(
    '#type' => 'weight',
    '#delta' => $num_rules,
    '#default_value' => -1 * $num_rules,
  );

  // Get commission structure for this product
  $result = db_query("SELECT * FROM {uc_affiliate2_product_commission} WHERE nid = :nid ORDER BY weight", array(':nid' => $node->nid));

  foreach ($result as $product_commission) {
    $pcid = $product_commission->pcid;
    $product_commission->commission_structure = unserialize($product_commission->commission_structure);
    $form['product_commissions'][$pcid]['weight'] = array(
      '#type' => 'weight',
      '#delta' => $num_rules,
      '#default_value' => $product_commission->weight,
    );
    $form['product_commissions'][$pcid]['rule'] = array(
      '#markup' => $rules[$product_commission->rule],
    );
    if (!empty($product_commission->rule_match)) {
      switch ($product_commission->rule) {
        case 'user':
          $account = user_load($product_commission->rule_match);
          $form['product_commissions'][$pcid]['match'] = array(
            '#markup' => theme('username', array('account' => $account)),
          );
          break;
        case 'role':
          $form['product_commissions'][$pcid]['match'] = array(
            '#markup' => user_role_load($product_commission->rule_match)->label,
          );
          break;
      }
    }
    $form['product_commissions'][$pcid]['commission_structure'] = array(
      '#markup' => implode(', ', $product_commission->commission_structure),
    );
    $form['product_commissions'][$pcid]['model'] = array(
      '#markup' => empty($product_commission->model) ? t('Any') : $product_commission->model,
    );
    $form['product_commissions'][$pcid]['delete'] = array(
      '#type' => 'checkbox',
    );
  }

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler for associating attributes with products or classes.
 */
function uc_affiliate2_product_commission_form_submit($form, &$form_state) {
  $nid = $form_state['values']['nid'];

  $product_commissions = $form_state['values']['product_commissions'];
  // save the new record
  if ($product_commissions['new']['commission_structure'] != '') {
    $commission = new stdClass();
    $commission->nid = $nid;
    $commission->model = $product_commissions['new']['model'];
    $commission->commission_structure = explode(',', $product_commissions['new']['commission_structure']);
    $commission->rule = $product_commissions['new']['rule'];
    $commission->rule_match = $product_commissions['new']['match'];
    if ($commission->rule == 'user') {
      $selected_user = user_load_by_name($commission->rule_match);
      $commission->rule_match = $selected_user->uid;
    }
    $commission->weight = $product_commissions['new']['weight'];
    uc_affiliate2_product_commission_save($commission);
  }

  // Delete the ones that have been checked.
  unset($product_commissions['new']);
  foreach ($product_commissions as $pcid => $comm) {
    if ($comm['delete'] == 1) {
      // Delete this record.
      uc_affiliate2_product_commission_delete($pcid);
    }
    elseif (!empty($comm['weight'])) {
      // Set the weight.
      db_update('uc_affiliate2_product_commission')
        ->fields(array(
          'weight'=>$comm['weight'],
        ))
        ->condition('pcid', $pcid)
        ->execute();
    }
  }
}

/**
 * Ajax form callback.
 */
function uc_affiliate2_ajax($form, $form_state) {
  return $form['product_commissions']['new']['match'];
}
