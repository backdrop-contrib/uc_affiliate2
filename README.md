# Ubercart Affiliate v2

This module provides affiliate functionality for Ubercart.

## Features

- User based tracking
- Role based tracking
- Monthly report of sales/commissions per affiliate in the admin section
- Ability to create affiliate links (both text and image)
- Product level link generation (e.g. generate a link for the actual product vs just the website), export to CSV or HTML
- Ability to add cookies so that affiliates get credit for return visits even if they don't go through original affiliate link
- Per product commissions
- Multiple levels of commissioning
- Fixed-rate and percentage commissions.
- Token and Rules integration.

## Notes

- For use directions and examples, please visit the [module's wiki](https://github.com/backdrop-contrib/uc_affiliate2/wiki) 
- The admin ( user uid: 1) can't be an affiliate
- Create a role, give it affiliate permission, and then assign users to that role.
If you assign two roles with affiliate permissions to a user, bad things happen :)
- When the user is assigned to multiple roles, the last one with affiliate permissions is taken as the current one.
- The path for affiliate links defaults to `/affiliate/[uid]`
  However, this can be overridden by adding the following line:
  `$conf['uc_affiliate2_path'] = 'my_affiliate_path';`
  in the site's settings.php file

## Current maintainer for Backdrop CMS version

- [argiepiano](https://github.com/argiepiano)

## Sponsors

### Backdrop CMS version sponsor:
- [scitexas.edu](scitexas.edu)

### Drupal version sponsors:
- Aaron Schiff (http://www.ecopaper.com/)
- Polish Your Image (http://polishyourimage.com)
- A Mountain Top, LLC (http://amountaintop.com)
- Health Curve, LLC (http://www.myhealthcurve.com) 

## Credits

- Created for Drupal by [bojanz](https://www.drupal.org/u/bojanz)
- Ported and expanded to Backdrop CMS by [argiepiano](https://github.com/argiepiano)

## License

This project is GPL v2 software. 