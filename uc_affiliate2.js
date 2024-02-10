(function($) {
  Backdrop.behaviors.uc_affiliate2 = {
    attach: function (context, settings) {
      $('#edit-products-all').once('uc_affiliate2').on('click', function() {
        new_value = false;

        if($('#edit-products-all').prop('checked')) {
          new_value = true;
        }

        $('.form-checkbox').each(function() {
          if(this.id != 'edit-products-all') {
            $('#' + this.id).prop('checked', new_value);
          }
        });
      });
    }
  }
})(jQuery);
