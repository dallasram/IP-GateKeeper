README.txt
==========

IP GateKeeper is a module that let's you completely ban both single IP-addresses as well as full ranges from sepecific paths on your site. The ban is triggered either at the bootstrap phase or Drupal initialization Phase.
Pros of boopstrap: you can get rid of unwanted visitors as early as possible without wasting server resources.
Pros of init: you can show the standard Drupal access denied message.


INSTALLATION
=============

Just enable the module as usual.


USAGE
============
After enabling the module, go to admin/config/people/ip_gatekeeper-ip-ranges to find form with three elements: 
"Path you wish to restrict"
"IP range start / Single IP-address"
"IP range end"

"Path you wish to restrict" is a specific path that you wish to only allow to known IPs.  There are some basic checks performed on this field, such as it is a valid path.

Two final two fields take an IP-Address in the form of "100.100.100.100". If the second field is filled, 
they be treated as a range. If you leave it empty, the value from the first field is only used.
(This is currently the only allowed range form, other types like bitmasks may come at later stage).

Caution! IP GateKeeper will also store any current aliases of the entered path but does not update path aliases if they change.
