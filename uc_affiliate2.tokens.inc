<?php
/**
 * @file
 *  Provides tokens for uc_affiliate2.
 */

/**
 * Implements hook_token_info().
 */
function uc_affiliate2_token_info() {
  $uc_order['affiliate-commission'] = array(
    'name' => t('Affiliate commission'),
    'description' => t('Ubercart affiliate commission data of the order.'),
    'type' => 'uc-affiliate-commission',
  );

  $types['uc-affiliate-commission'] = array(
    'name' => t('Affiliate commission'),
    'description' => t('Tokens for Ubercart Affiliate2 commission.'),
    'needs-data' => 'uc_order',
  );

  $uc_affiliate_commission['affiliate-commission-amount'] = array(
    'name' => t('Affiliate commission amount'),
    'description' => t('The affiliate commission amount for the order.'),
    'type' => 'uc_price'
  );

  $uc_affiliate_commission['affiliate'] = array(
    'name' => t('Affiliate'),
    'description' => t('The affiliate for the order.'),
    'type' => 'user'
  );

  $uc_affiliate_commission['affiliate-level'] = array(
    'name' => t('Affiliate level'),
    'description' => t('The level of affiliate for the order.'),
  );

  $uc_affiliate_commission['url'] = array(
    'name' => t('Affiliate commission link'),
    'description' => t('Link to the affiliate commissions.'),
  );

  return array(
    'types' => $types,
    'tokens' => array(
      'uc_order' => $uc_order, 
      'uc-affiliate-commission' => $uc_affiliate_commission,
    ),
  );
}

/**
 * Implements hook_tokens().
 */
function uc_affiliate2_tokens($type, $tokens, $data = array(), $options = array()) {
  if (isset($options['language'])) {
    $language_code = $options['language']->langcode;
  }
  $sanitize = !empty($options['sanitize']);

  $replacements = array();

  if ($type == 'uc-affiliate-commission' && !empty($data['uc_affiliate_commission'])) {
    $commission = $data['uc_affiliate_commission'];

    foreach ($tokens as $name => $original) {
      switch ($name) {
        case 'affiliate-commission-amount:formatted':
          $replacements[$original] = uc_currency_format($commission->commission);
          break;
        case 'affiliate-commission-amount:raw':
          $replacements[$original] = $commission->commission;
          break;
        case 'affiliate-level':
          $replacements[$original] = $commission->level;
          break;
        case 'url': 
          $replacements[$original] = url('user/' . $commission->aid . '/affiliate/commissions', array('absolute' => TRUE));
          break;
      }
    }
    // Handle chained tokens.
    if ($user_tokens = token_find_with_prefix($tokens, 'affiliate')) {
      $affiliate = user_load($commission->aid);
      $replacements += token_generate('user', $user_tokens, array('user' => $affiliate));
    }
  }

  if ($type == 'uc_order' && !empty($data['uc_order'])) {
    if ($affiliate_tokens = token_find_with_prefix($tokens, 'affiliate-commission')) {
      $order = $data['uc_order'];
      $affiliate_commission = uc_affiliate2_commission_load_from_order($order->order_id);
      $replacements += token_generate('uc-affiliate-commission', $affiliate_tokens, array('uc_affiliate_commission' => $affiliate_commission));
    }
  }

  return $replacements;
}
