<?php

/**
 * @file
 * Views handler: Affiliate2 commission structure field.
 */

/**
 * Return a value to display in the View.
 */
class uc_affiliate2_handler_field_commission_structure extends views_handler_field {
  function render($values) {
    $value = $this->get_value($values);
    if (!empty($value)) {
      return implode(', ', unserialize($value));
    }
  }
}
