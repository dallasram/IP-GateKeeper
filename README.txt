README.txt
==========

IP GateKeeper is a module that lets specific paths only be accessible from either a single IP-address or full IP range. All other IPs would not be allowed to access those paths.  An example usage would be the /user, /user/login, and /user/password pages could only be accessible from within your organization's IP range.

Caution! IP-based restrictions are not fool-proof, and as a stand-alone solution IP GateKeeper is a weak security measure.  It is recommended that you use IP GateKeeper within the context of a proper security architecture.

The IP check is triggered either at the bootstrap phase or Drupal initialization Phase.
Pros of boopstrap: you can get rid of unwanted visitors as early as possible without wasting server resources.
Pros of init: you can show the standard Drupal access denied message.


INSTALLATION
=============

Just enable the module as usual.


USAGE
============
After enabling the module, go to admin/config/people/ip_gatekeeper-ip-ranges to find a form with three elements:
"Path you wish to restrict"
"IP range start / Single IP-address"
"IP range end"

"Path you wish to restrict" is a specific path that you wish to only allow to known IPs.  There are some basic checks performed on this field, such as it is a valid path.

Two final two fields take an IP-Address in the form of "100.100.100.100". If the second field is filled,
they be treated as a range. If you leave it empty, the value from the first field is only used.
(This is currently the only allowed range form, other types like bitmasks may come at later stage).

Caution! IP GateKeeper will also store any current aliases of the entered path but does not update path aliases if they change.
