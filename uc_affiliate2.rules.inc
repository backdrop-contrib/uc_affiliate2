<?php

/**
 * Implements hook_rules_data_info().
 */
function uc_affiliate2_rules_data_info() {
  return array(
    'uc_affiliate2_commission' => array(
      'label' => t('Affiliate commission'),
      'wrap' => TRUE,
      'group' => t('UC Affiliate2'),
      'property info' => array(
        'commission_id' => array(
          'type' => 'integer',
          'label' => t('Commission ID'),
        ),
        'order_id' => array(
          'type' => 'integer',
          'label' => t('Order ID'),  
        ),
        'aid' => array(
          'type' => 'integer',
          'label' => t('Affiliate ID'),
        ),
        'affiliate' => array(
          'type' => 'user',
          'label' => t('Affiliate'),
          'getter callback' => 'uc_affiliate2_affiliate_load',
        ),
        'commission' => array(
          'type' => 'decimal',
          'label' => t('Affiliate commission'),
        ),
        'level' => array(
          'type' => 'integer',
          'label' => t('Affiliate level'),
        ),
        'commission_notes' => array(
          'type' => 'text',
          'label' => t('Affiliate commission notes'),
        ),
        'order' => array(
          'type' => 'uc_order',
          'label' => t('Order'),
        ),
      ),
    ),
  );
}

/**
 * Implements hook_rules_condition_info().
 */
function uc_affiliate2_rules_condition_info() {
  $items['uc_affiliate2_user_has_affiliate'] = array(
    'label' => t('User has affiliate'),
    'parameter' => array(
      'user' => array(
        'type' => 'user',
        'label' => t('User'),
      ),
    ),
    'group' => t('User account'),
  );
  $items['uc_affiliate2_condition_commission_amount'] = array(
    'label' => t('Commission amount comparison'),
    'group' => t('UC Affiliate2'),
    'parameter' => array(
      'commission' => array(
        'type' => 'decimal',
        'label' => t('Affiliate commission'),
      ),
      'operator' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'options list' => 'uc_order_condition_value_operator_options',
      ),
      'amount' => array(
        'type' => 'decimal',
        'label' => t('Amount'),
        'restriction' => 'input',
      ),
    ),
  );
  $items['uc_affiliate2_affiliate_level'] = array(
    'label' => t('Check affiliate level'),
    'group' => t('UC Affiliate2'),
    'parameter' => array(
      'uc_affiliate2_commission' => array(
        'type' => 'uc_affiliate2_commission',
        'label' => t('Affiliate commission'),
      ),
      'operator' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'options list' => 'uc_order_condition_value_operator_options',
      ),
      'amount' => array(
        'type' => 'integer',
        'label' => t('Level'),
        'restriction' => 'input',
      ),
    ),

  );
  return $items;
}

/**
 * Implements hook_rules_event_info().
 */
function uc_affiliate2_rules_event_info() {
  $items = array(
    'uc_affiliate2_apply_commission' => array(
      'label' => t('Affiliate commissions being applied'),
      'group' => t('UC Affiliate2'),
      'variables' => array(
        'uc_affiliate2_commission' => array(
          'type' => 'uc_affiliate2_commission',
          'label' => t('Affiliate commission'),
        ),
        'uc_order' => array(
          'type' => 'uc_order',
          'label' => t('Order'),
        ),
      ),
    ),
  );
  return $items;
}

/**
 * Implements hook_rules_action_info().
 */
function uc_affiliate2_rules_action_info() {
  $items = array(
    'uc_affiliate2_set_commission' => array(
      'label' => t('Override the commission percentage'),
      'group' => t('UC Affiliate2'),
      'save' => TRUE,
      'parameter' => array(
        'uc_order' => array(
          'type' => 'uc_order',
          'label' => t('Order'),
        ),
        'uc_affiliate2_commission' => array(
          'type' => 'uc_affiliate2_commission',
          'label' => t('UC Affiliate2 commission'),
        ),
        'percentage' => array(
          'type' => 'decimal',
          'label' => t('Percentage'),
          'description' => t('Enter the percentage to use to override the default commission percentage.'),
          'restriction' => 'input',
        ),
      ),
    )
  );
  return $items;
}

/**
 * Condition: check if the user has an affiliate.
 */
function uc_affiliate2_user_has_affiliate($user) {
  // Check session variable and the database for affiliate for the user making
  // the purchase.
  if (isset($_SESSION['affiliate']) && $_SESSION['affiliate'] > 0) {
    return TRUE;
  }
  if (_uc_affiliate2_get_user($user->uid) > 0) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Condition: commission amount comparison.
 */
function uc_affiliate2_condition_commission_amount($commission, $operator, $amount) {
  switch ($operator) {
    case 'less':
      return $commission < $amount;
    case 'less_equal':
      return $commission <= $amount;
    case 'equal':
      return $commission == $amount;
    case 'greater':
      return $commission > $amount;
    case 'greater_equal':
      return $commission >= $amount;
  }
}

/**
 * Condition: level comparison.
 */
function uc_affiliate2_affiliate_level($commission, $operator, $level) {
  switch ($operator) {
    case 'less':
      return $commission->level < $level;
    case 'less_equal':
      return $commission->level <= $level;
    case 'equal':
      return $commission->level == $level;
    case 'greater':
      return $commission->level > $level;
    case 'greater_equal':
      return $commission->level >= $level;
  }
}

/**
 * Action: Override the commission percentage.
 */
function uc_affiliate2_set_commission($order, $uc_affiliate2_commission, $percentage) {
  $uc_affiliate2_commission->commission = $order->order_total * $percentage / 100;
}
