<?php

/**
 * @file
 * Views hooks and callback registries.
 */

/**
 * Implementation of hook_views_data().
 */
function uc_affiliate2_views_data() {
  $data['uc_affiliate2_product_commission']['table']['group'] = t('UC Affiliate2');

  $data['uc_affiliate2_product_commission']['table']['join'] = array(
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
    ),
  );

  $data['uc_affiliate2_product_commission']['commission_structure'] = array(
    'title' => t('Product Commission Structure'),
    'help' => t('The product commission structure.'),
    'field' => array(
      'handler' => 'uc_affiliate2_handler_field_commission_structure',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['uc_affiliate2_commission']['table']['group'] = t('UC Affiliate2 Order');

  $data['uc_affiliate2_commission']['table']['join'] = array(
    'uc_orders' => array(
      'left_field' => 'order_id',
      'field' => 'order_id',
    ),
    'users' => array(
      'left_field' => 'uid',
      'field' => 'aid',
    ),
  );

  $data['uc_affiliate2_commission']['aid'] = array(
    'title' => t('Affiliate ID'),
    'help' => t('The ID of the affilite user.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'relationship' => array(
      'base' => 'users',
      'base field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('Affiliate User'),
    ),
  );
  $data['uc_affiliate2_commission']['commission'] = array(
    'title' => t('Order Commission'),
    'help' => t('The order affiliate commission structure.'),
    'field' => array(
      'handler' => 'uc_order_handler_field_money_amount',
      'click sortable' => TRUE,
      'float' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );
  $data['uc_affiliate2_commission']['commission_notes'] = array(
    'title' => t('Order Commission notes'),
    'help' => t('The order affiliate commission notes.'),
    'field' => array(
      'handler' => 'views_handler_field',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  return $data;
}
