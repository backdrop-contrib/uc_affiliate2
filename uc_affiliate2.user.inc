<?php

/**
 * @file
 * User functions for the uc_affiliate2 module.
 *
 */

/*
 * Display dashboard for affiliate
 */
function uc_affiliate2_dashboard($account) {
  $commissions = array(
    'today' => 0,
    'week' => 0,
    'year' => 0,
  );
  $clicks = array(
    'today' => 0,
    'week' => 0,
    'year' => 0,
  );

  // Get today's commissions.
  $dates_today = _uc_affiliate2_predefined_range(7);
  $sql = 'SELECT COUNT(ac.commission_id) as cnt
            FROM {uc_affiliate2_commission} ac
            INNER JOIN {uc_orders} uco on ac.order_id = uco.order_id
            WHERE aid = :aid AND uco.created
            BETWEEN :start AND :end';

  $result = db_query($sql, array(':aid' => $account->uid, ':start' => $dates_today['start'], ':end' => $dates_today['end']));

  foreach ($result as $row) {
     $commissions['today'] += $row->cnt;
  }

  // Get commissions from the last 7 days.
  $dates_week = _uc_affiliate2_predefined_range(6);

  $sql = 'SELECT COUNT(ac.commission_id) as cnt
            FROM {uc_affiliate2_commission} ac
            INNER JOIN {uc_orders} uco on ac.order_id = uco.order_id
            WHERE aid = :aid AND uco.created
            BETWEEN :start AND :end';

  $result = db_query($sql, array(':aid' => $account->uid, ':start' => $dates_week['start'], ':end' => $dates_week['end']));

  foreach ($result as $row) {
    $commissions['week'] += $row->cnt;
  }

  // Get commissions from the last year
  $dates_year = _uc_affiliate2_predefined_range(5);

  $sql = 'SELECT COUNT(ac.commission_id) as cnt
            FROM {uc_affiliate2_commission} ac
            INNER JOIN {uc_orders} uco on ac.order_id = uco.order_id
            WHERE aid = :aid AND uco.created 
            BETWEEN :start AND :end';

  $result = db_query($sql, array(':aid' => $account->uid, ':start' => $dates_year['start'], ':end' => $dates_year['end']));

  foreach ($result as $row) {
    $commissions['year'] += $row->cnt;
  }

  // Get today's clicks.
  $sql = "SELECT count(click_id) as clicks from {uc_affiliate2_clicks}
            WHERE aid = :aid AND time BETWEEN :start AND :end
            GROUP BY aid";
  $result = db_query($sql, array(':aid' => $account->uid, ':start' => $dates_today['start'], ':end' => $dates_today['end']));

  foreach ($result as $row) {
    $clicks['today'] += $row->clicks;
  }

  // Get commissions from the last 7 days.
  $sql = "SELECT count(click_id) as clicks from {uc_affiliate2_clicks}
            WHERE aid = :aid AND time BETWEEN :start AND :end
            GROUP BY aid";
  $result = db_query($sql, array(':aid' => $account->uid, ':start' => $dates_week['start'], ':end' => $dates_week['end']));

  foreach ($result as $row) {
    $clicks['week'] += $row->clicks;
  }

  // Get commissions from the last year.
  $sql = "SELECT count(click_id) as clicks from {uc_affiliate2_clicks}
            WHERE aid = :aid AND time BETWEEN :start AND :end
            GROUP BY aid";
  $result = db_query($sql, array(':aid' => $account->uid, ':start' => $dates_year['start'], ':end' => $dates_year['end']));

  foreach ($result as $row) {
    $clicks['year'] += $row->clicks;
  }

  backdrop_set_title(t('Affiliate Center'));

  $output = '';
  $output .= '<div id="affiliate-sales-header"><h2>' . t('Commission History') . '</h2> <br />';

  $output .=  t('Sales Today:') . " $commissions[today] <br />" .
              t('Sales last 7 Days:') . " $commissions[week] <br />" .
              t('Last 365 Days:') . " $commissions[year] <br /><br />";

  $commissions_url = implode('/', array('user', $account->uid, 'affiliate', 'commissions'));
  $output .= l(t('Click here for more reports.'), $commissions_url, array('query' => NULL, 'fragment' => NULL, 'absolute' => TRUE, 'html' => TRUE));

  $output .= '</div>';

  $output .= '<br /><div id="affiliate-sales-click-count"><h2>' . t('Unique Click Count') . '</h2> <br />';

  $output .= t("Today's Clicks:") . " $clicks[today] <br />" .
             t("Last 7 Days:") . " $clicks[week] <br />" .
             t("Last 365 Days:") . " $clicks[year] <br /><br />";

  $clicks_url = implode('/', array('user', $account->uid, 'affiliate', 'clicks'));
  $output .= l(t('Click here for customized click reporting.'), $clicks_url, array('query' => NULL, 'fragment' => NULL, 'absolute' => TRUE, 'html' => TRUE));

  $output .= '</div>';

  $output .= '<br /><div id="affiliate-links-header"><h2>' . t('Affiliate Links') . '</h2>';
  switch (config_get('uc_affiliate2.settings', 'affiliate_default_link_type')) {
    case 'path':
      $output .= t('You can append a destination page to the end of the affiliate links to redirect to a specific page,
                  as well as track clicks to it, i.e.
                  @example', array('@example' => _uc_affiliate2_url('node/1', $account, array('alias' => TRUE, 'type' => 'path'))));
      break;
    case 'query':
      $output .= t('You can link to any page on this site by adding your query argument (?a=@uid) to the end of the url, i.e. @example', array('@uid' => $account->uid, '@example' => _uc_affiliate2_url('node/1', $account, array('alias' => TRUE, 'type' => 'query'))));
      break;
  }

  $text_link = config_get('uc_affiliate2.settings', 'affiliate_text_link_template');
  $text_link = _uc_affiliate2_l($text_link, '<front>', $account, array('html' => TRUE));
  $text_link = array(
    '#type' => 'textarea',
    '#title' => t('Text Link'),
    '#value' => $text_link,
  );
  $output .= backdrop_render($text_link);

  $output .= _uc_affiliate2_display_banners();

  $links_url = implode('/', array('user', $account->uid, 'affiliate', 'links'));
  $output .= '<p>' . l(t('Click here for product links.'), $links_url, array('query' => NULL, 'fragment' => NULL, 'absolute' => TRUE, 'html' => TRUE)) . '</p>';

  $output .= '</div>';

  return $output;
}

/**
 * Callback page to generate links.
 */
function uc_affiliate2_links($account) {
  $output = '';

  $products = array('all' => t('Show all'));
  $products += _uc_affiliate2_get_product_list();

  if (count($products) > 1) {
    $output .= '<b>' . t('Export product feed') . '</b> <br />';
    $output .= t('Choose the products which you want included in your feed.') . '<br />';
    $output .= t('All product links will have your affiliate id included.') . '<br /><br /> ';
    $form = backdrop_get_form('_uc_affiliate2_product_form', $products, $account);
    $output .= backdrop_render($form);
  }
  else {
    $output .= t('No products found.');
  }

  return $output;
}

/**
 * Export the products as html.
 */
function uc_affiliate2_links_export($user) {
  global $base_url;
  $ids = func_get_args();

  // Remove the user object from the ids.
  array_shift($ids);

  $output = '<br />';
  $output .= t('!image is the placeholder image in case none is present.',
                  array('!image' => '<img src="' . $base_url . '/core/misc/watchdog-error.png' . '" />'));
  $output .= '<br />';

  if ($ids[0] == 'all') {
    $products = _uc_affiliate2_get_products($user, 'html');
  }
  else {
    $products = _uc_affiliate2_get_products($user, 'html', $ids);
  }

  $output .= _uc_affiliate2_display_products($products);
  $output .= theme('pager', array('tags' => NULL, 'element' => 0));

  return $output;
}

/**
 * Show the html products in a table.
 */
function _uc_affiliate2_display_products($products) {
  global $base_url;

  $header[] = array(
    'field' => 'product_title',
    'data' => t('Product Title'),
  );
  $header[] = array(
    'field' => 'preview',
    'data' => t('Preview'),
  );
  $header[] = array(
    'field' => 'code',
    'data' => t('Code'),
  );

  if (count($products) < 1) {
    return t('No products found.');
  }
  $rows=array();
  foreach ($products as $key => $product) {
    $line = array();

    $line[] = $key;
    $line[] = $product;
    $line[] = '<textarea cols="60" rows="4">' . $product . '</textarea>';

    $rows[] = $line;
  }
  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Form builder to export products.
 */
function _uc_affiliate2_product_form($form, &$form_state, $products, $account) {

  $form['#attached']['js'] = array(
    backdrop_get_path('module', 'uc_affiliate2') . '/js/uc_affiliate2.js'
  );

  $form_state['account'] = $account;

  $form['uid'] = array(
    '#type' => 'value',
    '#value' =>  $account->uid,
  );

  $form['products'] = array(
    '#title' => t('Products'),
    '#type' => 'checkboxes',
    '#required' => TRUE,
    '#options' => $products,
  );

  $form['export_type'] = array(
    '#type' => 'radios',
    '#title' => t('Export as'),
    '#options' => array(
      'html' => 'HTML',
      'csv' => 'CSV',
    ),
    '#default_value' => 'csv',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export'),
  );

  return $form;
}

/**
 * Submit handler for product export form.
 */
function _uc_affiliate2_product_form_submit($form, &$form_state) {
  $user = $form_state['account'];

  $show_these = array();

  // build the filter array for _uc_affiliate2_get_products from the form checkboxes
  foreach ($form_state['values']['products'] as $product_id => $status) {
    if ($status !== 0) {
      $show_these[] = $product_id;
    }
  }

  if (in_array('all', $show_these)) {
    $show_these = array('all'); // we don't need anything else
  }

  if ($form_state['values']['export_type'] == 'csv') {
    backdrop_add_http_header('Content-Type',"application/csv; charset=\"utf-8\"\n");
    backdrop_add_http_header('Content-Disposition','attachment; filename="export.csv"');

    if ($show_these[0] == 'all') {
      print _uc_affiliate2_get_products($user, 'csv');
    }
    else {
      print _uc_affiliate2_get_products($user, 'csv', $show_these);
    }

    backdrop_exit();
  }
  else { // html
    $ids = implode('/', $show_these);
    $path = implode('/', array('user', $form_state['values']['uid'], 'affiliate', 'links', 'export', $ids));

    backdrop_goto($path);
  }
}

/**
 * Display banners.
 */
function _uc_affiliate2_display_banners() {
  global $base_url, $user;

  $banners = config_get('uc_affiliate2.settings', 'affiliate_banner_template');

  $header[] = array(
    'field' => 'banner_preview',
    'data' => t('Banner Preview'),
  );
  $header[] = array(
    'field' => 'banner_code',
    'data' => t('Banner Code'),
  );

  $banners = explode('<', $banners);
  unset($banners[0]);

  if (count($banners) < 1) {
    return;
  }

  foreach ($banners as $banner) {
    $banner = '<' . $banner;
    $banner_link = _uc_affiliate2_l($banner, '<front>', $user, array('html' => TRUE));

    $line = array();
    $line[] = $banner;
    $line[] = '<textarea cols="45">' . $banner_link . '</textarea>';
    $rows[] = $line;
  }

  return theme('table', array('header' => $header, 'rows' => $rows));
}

/*
 * Display affiliate commission records.
 */
function uc_affiliate2_commissions($account) {
  backdrop_set_title(t('Commissions'));

  // setup the table
  $header = array(
    array('data' => t('Customer')),
    array(
      'data' => t('Order'),
      'field' => 'order_id',
      'sort' => 'desc',
    ),
    array(
      'data' => t('Date'),
      'field' => 'order_date',
    ),
    array('data' => t('Notes')),
    array(
      'data' => t('Purchase'),
      'field' => 'order_total',
    ),
    array(
      'data' => t('Commission'),
      'field' => "commission",
    ),
  );

  // Create sql to affiliate's commissions.
  $dates = _uc_affiliate2_predefined_range(4);

  $query = db_select('uc_affiliate2_commission', 'ac');
  $query->extend('TableSort')
  ->orderByHeader($header)
  ->innerJoin( 'uc_orders', 'uco', 'ac.order_id = uco.order_id' );
  $query->leftJoin( 'users', 'u', 'uco.uid = u.uid');
  $query->fields( 'ac', array('order_id', 'commission_notes', 'commission'));
  $query->fields( 'uco', array('uid', 'created', 'order_total'));
  $query->fields( 'u', array('name'));
  $query->condition('aid', $account->uid, '=');
  $query->condition('uco.created', array($dates['start'], $dates['end']), 'BETWEEN' );

  $result = $query->execute();

  $total_sales = 0;
  $total_commission = 0;
  $rows = array();
  foreach ($result as $commission) {
    if ($commission->uid > 0 && user_access('access user profiles')) {
      // $commission['name'] represents the name that you can see when visiting the profile
      $u = l($commission->name, 'user/' . $commission->uid);
    }
    else {
      // In this case, we only show a number
      if ($commission->uid == 0) {
        $u = t('Unknown');
      }
      else {
        $u = t('User #') . $commission->uid;
      }
    }
    $rows[] = array(
      $u,
      $commission->order_id,
      format_date($commission->created, 'custom', 'Y/m/d'),
      $commission->commission_notes,
      uc_currency_format($commission->order_total),
      uc_currency_format($commission->commission),
    );
    $total_sales += $commission->order_total;
    $total_commission += $commission->commission;
  }
  if ($total_commission > 0) {
    $rows[] = array('', '', '', t('Total:'), uc_currency_format($total_sales), uc_currency_format($total_commission));
  }
  $empty_message = t('No commision has been paid for the period starting');
  $empty_message .= ' ' . format_date($dates['start'], 'custom', 'F d, Y') . ' ';
  $empty_message .= t('and ending');
  $empty_message .= ' ' . format_date($dates['end'], 'custom', 'F d, Y');
  $output = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => $empty_message));

  return $output;
}

/*
 * Display affiliate clicks.
 */
function uc_affiliate2_clicks($account) {
  backdrop_set_title(t('Unique Click count'));
  $dates = _uc_affiliate2_predefined_range(arg(4));
  $query = db_select('uc_affiliate2_clicks')
  ->fields('uc_affiliate2_clicks', array('time'))
  ->condition('aid', $account->uid)
  ->condition('time', array($dates['start'], $dates['end']), 'BETWEEN');
  $header = array(
    array(
      'data' => t('Date'),
      'field' => "time",
    ),
    array(
      'data' => t('Unique Clicks'),
      'field' => "clicks",
    ),
  );
  $query->extend('TableSort')->orderByHeader($header);
  $result = $query->execute();
  $clicks = array();
  foreach ($result as $click) {
    $date = date('Y/m/d', $click->time);
    if(!isset($clicks[$date])) $clicks[$date]=0;
    $clicks[$date] += 1;
  }
  $total_clicks = 0;
  foreach ($clicks as $click_str => $clicks_count) {
    $click_date = strtotime($click_str);
    $click_date_end = $click_date + 86399;
    $link = l(date('Y/m/d', $click_date), 'user/' . $account->uid . '/affiliate/clicks/details/' . $click_date . '/' . $click_date_end);
    $rows[] = array($link, $clicks_count);
    $total_clicks += $clicks_count;
  }
  $rows[] = array(t('Total:'), $total_clicks);

  // Build the output

  $output = theme('table', array('header'=>$header, 'rows'=> $rows));

  return $output;
}

/*
 * Display a detailed clicks view for the selected date.
 */
function uc_affiliate2_click_details($account) {
  backdrop_set_title(t('Unique Clicks - Details'));

  $cookie_statuses = array(t('Not sent'), t('Sent (Unconfirmed)'), t('Sent (Confirmed)'));

  $output = t('Viewing clicks for:') . ' ' . date('Y/m/d', arg(5)) . '<br /> <br />';
  $output .= t('Cookie Status can be: Not sent, Sent (Unconfirmed), Sent (Confirmed).') . '<br />';

  $ip_masking = config_get('uc_affiliate2.settings', 'affiliate_ip_masking');

  // Setup the table
  $header = array(
    array(
      'data' => t('Date/Time'),
      'field' => "time",
      'sort' => 'desc',
    ),
    array(
      'data' => t('Client IP'),
      'field' => "client_ip",
    ),
    array(
      'data' => t('Referrer'),
      'field' => "referrer",
    ),
    array(
      'data' => t('Destination'),
      'field' => "destination",
    ),
    array(
      'data' => t('Cookie Status'),
      'field' => "cookie_status",
    ),
  );

  if ($ip_masking == 'hide') {
    unset($header[1]);
  }

  // Create sql to display level of affiliate specified.
  $dates = _uc_affiliate2_predefined_range(0);

  $sql = "SELECT * FROM {uc_affiliate2_clicks} WHERE aid = :aid AND time BETWEEN :start AND :end";

  $result = db_query($sql, array(':aid'=>$account->uid, ':start'=>$dates['start'],':end'=> $dates['end']));

  $total_clicks = 0;
  while ($click = $result->fetchAssoc()) {
    if ($click['referrer']) {
      $click['referrer'] = l(t('Referrer'), $click['referrer']); // make it a link
    }
    else {
      $click['referrer'] = t('N/A');
    }

    if ($click['destination']) {
      $click['destination'] = l(t('Destination'), $click['destination']); // make it a link
    }
    else {
      $click['destination'] = t('Frontpage');
    }

    if ($click['client_ip']) {
      $click['client_ip'] = _uc_affiliate2_mask_ip($click['client_ip']);
    }
    else {
      $click['client_ip'] = t('N/A');
    }

    $row = array(
      format_date($click['time'], 'custom', 'Y/m/d H:i:s'),
      $click['client_ip'],
      $click['referrer'],
      $click['destination'],
      ($click['cookie_status'] === NULL) ? 'Not specified' : $cookie_statuses[$click['cookie_status']],
    );

    if ($ip_masking == 'hide') {
      unset($row[1]);
    }

    $rows[] = $row;
    $total_clicks += 1;
  }
  $rows[] = array(
    array(
      'data' => t('Total:'),
      'colspan' => ($ip_masking == 'hide' ? 3 : 4),
      'align' => 'right',
    ),
    $total_clicks,
  );

  $output .= theme('table', array('header' => $header, 'rows' => $rows));

  return $output;
}

/**
 * Mask the IP address per the defined setting.
 */
function _uc_affiliate2_mask_ip($ip) {
  global $user;

  if ($user->uid == 1) {
    return $ip;
  }

  $ip_masking = config_get('uc_affiliate2.settings', 'affiliate_ip_masking');
  $ip_numbers = explode('.', $ip);

  switch ($ip_masking) {
    default: // = case 'hide':
      $ip_numbers[3] = '*';
    case 'one':
      $ip_numbers[2] = '*';
    case 'two':
      $ip_numbers[1] = '*';
      $ip_numbers[0] = '*';
      break;

    case 'first':
      $ip_numbers[1] = '*';
      $ip_numbers[2] = '*';
      $ip_numbers[3] = '*';
      break;

    case 'full':
      break;
  }

  $ip = implode('.', $ip_numbers);

  return $ip;
}

/**
 * Display affiliate orders by status.
 */
function uc_affiliate2_orders($account) {
  backdrop_set_title(t('All Orders'));

  // Setup the table.
  $header = array(
    array('data' => t('Customer')),
    array(
      'data' => t('Order'),
      'field' => 'ao.order_id',
      'sort' => 'desc',
    ),
    array(
      'data' => t('Order Status'),
      'field' => 'order_status',
    ),
    array(
      'data' => t('Date'),
      'field' => 'created',
    ),
    array(
      'data' => t('Purchase'),
      'field' => 'order_total',
    ),
  );

  // Create sql to affiliate's commissions
  $query = db_select('uc_affiliate2_orders', 'ao');
  $query->innerJoin('uc_orders', 'uco', 'uco.order_id = ao.order_id');
  $query->leftJoin('users', 'u', 'u.uid = uco.uid');
  $query->fields('uco')
  ->fields('ao', array('order_id'))
  ->fields('u', array('name'))
  ->condition('ao.aid', $account->uid);
  $query
  ->extend('TableSort')
  ->orderByHeader($header);
  $result = $query->execute();

  $total_sales = 0;
  foreach ($result as $order) {
    if ($order->uid > 0 && user_access('access user profiles')) {
      $u = l($order->name, 'user/' . $order->uid);
    }
    else {
      // in this case, we only show a number
      if ($order->uid == 0) {
        $u = t('Guest');
      }
      else {
        $u = t('User #') . $order->uid;
      }
    }
    $total_sales += $order->order_total;
    $rows[] = array(
      $u,
      $order->order_id,
      ucwords(str_replace('_', ' ', $order->order_status)),
      format_date($order->created, 'custom', 'm-d-Y'),
      uc_currency_format($order->order_total),
    );
  }
  $rows[] = array('', '', '', t('Total:'), uc_currency_format($total_sales));
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  return $output;
}

/*
 * Display affiliate's downline.
 */
function uc_affiliate2_downline($account, $level = 1) {
  backdrop_set_title(t('Downline'));

  // Make sure user doesn't try to manipulate depth.
  if ($level > config_get('uc_affiliate2.settings', 'affiliate_hierarchy_depth')) {
    $level = config_get('uc_affiliate2.settings', 'affiliate_hierarchy_depth');
  }

  // setup the table
  $header = array(
    array(
      'data' => t('Username'),
      'field' => "u$level.name",
    ),
    array(
      'data' => t('Affiliate'),
      'field' => "u$level.status",
    ),
    array(
      'data' => t('Member for'),
      'field' => "u$level.created",
      'sort' => 'desc',
    ),
    array(
      'data' => t('Last access'),
      'field' => "u$level.access",
    ),
    array('data' => t('Level')),
    array(
      'data' => t('Customers'),
      'field' => "uc.user_count",
    ),
  );

  // Create sql to display level of affiliate specified
  $query = db_select('users', 'u1')
  ->fields('u1', array('uid'));
  $next = 1;
  for ($i = 1; $i <= $level; $i++) {
    $next++;
    $query->innerJoin('uc_affiliate2_users', 'au' . $i, 'au' . $i . '.uid = u' . $i . '.uid');
    $query->innerJoin('users', 'u' . $next, 'au' . $i . '.aid = u' . $next . '.uid');
  }
  // user_count subquery
  $subquery = db_select('uc_affiliate2_users', 'au');
  $subquery->addField('au', 'aid');
  $subquery->addExpression('COUNT(au.aid)', 'user_count');
  $subquery->groupBy('au.aid');
  // Attach subquery & field
  $query->leftJoin($subquery, 'uc', 'uc.aid = u1.uid');
  $query->addField('uc', 'user_count');

  $query->condition('u' . $next . '.uid', $account->uid, '=');
  $query->extend('PagerDefault')->limit(25)->extend('TableSort')->orderByHeader($header);

  $result = $query->execute();

  // Create the data rows for our table
  $rows = array();
  foreach ($result as $affiliate) {
    $downline_user = user_load($affiliate->uid);
    $rows[] = array(
      theme('username', array('account' => $downline_user) ),
      user_access('act as affiliate', $downline_user) ? 'Yes' : 'No',
      format_interval(REQUEST_TIME - $downline_user->created),
      $downline_user->access ? t('%time ago', array('%time' => format_interval(REQUEST_TIME - $downline_user->access))) : t('never'),
      $level,
      $affiliate->user_count > 0 ? $affiliate->user_count : 0,
    );
  }

  // Build the output.
  $output = '<ul class="tabs secondary">';
  for ($i = 1; $i <= config_get('uc_affiliate2.settings', 'affiliate_hierarchy_depth'); $i++) {
    $output .= ($i == $level ? '<li class="active">' : '<li>');
    $output .= l(t("Level") . " " . $i, 'user/' . $account->uid . '/affiliate/downline/' . $i) . "</li>";
  }
  $output .= "</ul>";

  $output .= '<h2>' . t('Total Level !level', array('!level' => $level)) . '</h2>';
  $output .= theme('table',
    array(
    'header' => $header,
    'rows' => $rows,
    'empty' => t('Currently you have nobody in your downline.'),
  )
  );
  $output .= theme('pager', array('tags' => array()));
  return $output;
}

/**
 * Display affiliate's upline.
 */
function uc_affiliate2_upline($account) {
  backdrop_set_title(t('Upline'));

  // setup the table
  $header = array(
    array('data' => t('Level')),
    array('data' => t('Username')),
    array('data' => t('Member for')),
    array('data' => t('Last access')),
    array('data' => t('Customers')),
  );

  // Create sql to display user's upline
  $level = config_get('uc_affiliate2.settings', 'affiliate_hierarchy_depth');
  $rows = array();
  $next_uid = $account->uid;
  for ($i = 1; $i <= $level; $i++) {
    $upline = db_select('users', 'u')
    ->condition('au.uid', $next_uid, '=')
    ->fields('u', array('uid', 'name', 'status', 'access') );
    $upline->innerJoin('uc_affiliate2_users', 'au', 'au.uid = u.uid');
    $upline->execute();

    // create the data rows for our table
    $status = array(t('blocked'), t('active'));
    foreach ($upline as $affiliate) {
      $next_uid = $affiliate->uid;
      $user_count = db_select('uc_affiliate2_users', 'au')
      ->condition('au.uid', $next_uid, '=')
      ->countQuery()->execute()->fetchfield();
      $rows[] = array(
        $affiliate->level,
        theme('username', array('account'=>$affiliate)),
        format_interval(REQUEST_TIME - $affiliate->created),
        $affiliate->access ? t('%time ago', array('%time' => format_interval(REQUEST_TIME - $account->access))) : t('never'),
        $user_count ? $user_count : 0,
      );
    }
  }

  if (count($rows) > 0) {
    $output = theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t("You are a top level affiliate.  There are no affiliates in your upline.")));
  }
  else {
    $output = "<p>" . t("You are a top level affiliate.  There are no affiliates in your upline.") . "</p>";
  }
  return $output;
}
