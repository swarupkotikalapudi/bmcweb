# OpenBMC webserver #

This component attempts to be a "do everything" embedded webserver for openbmc.


## Capabilities ##
At this time, the webserver implements a few interfaces:
+ Authentication middleware that supports cookie and token based authentication, as well as CSRF prevention backed by linux PAM authentication credentials.
+ An (incomplete) attempt at replicating phosphor-dbus-rest interfaces in C++.  Right now, a few of the endpoint definitions work as expected, but there is still a lot of work to be done.  The portions of the interface that are functional are designed to work correctly for phosphor-webui, but may not yet be complete.
+ Replication of the rest-dbus backend interfaces to allow bmc debug to logged in users.
+ An initial attempt at a read-only redfish interface.  Currently the redfish interface targets ServiceRoot, SessionService, AccountService, Roles, and ManagersService.  Some functionality here has been shimmed to make development possible.  For example, there exists only a single user role.
+ SSL key generation at runtime.  See the configuration section for details.
+ Static file hosting.  Currently, static files are hosted from the fixed location at /usr/share/www.  This is intended to allow loose coupling with yocto projects, and allow overriding static files at build time.
+ Dbus-monitor over websocket.  A generic endpoint that allows UIs to open a websocket and register for notification of events to avoid polling in single page applications.  (this interface may be modified in the future due to security concerns.

## Configuration

BMCWeb is configured by setting `-D` flags that correspond to options
in `bmcweb/meson_options.txt` and then compiling.  For example, `meson
<builddir> -Dkvm=disabled ...` followed by `ninja` in build directory.
The option names become C++ preprocessor symbols that control which code
is compiled into the program.

### Compile bmcweb with default options:
```ascii
meson builddir
ninja -C builddir
```
### Compile bmcweb with yocto defaults:
```ascii
meson builddir -Dbuildtype=minsize -Db_lto=true -Dtests=disabled
ninja -C buildir
```
If any of the dependencies are not found on the host system during
configuration, meson automatically gets them via its wrap dependencies
mentioned in `bmcweb/subprojects`.

### Enable/Disable meson wrap feature
```ascii
meson builddir -Dwrap_mode=nofallback
ninja -C builddir
```
### Enable debug traces
```ascii
meson builddir -Dbuildtype=debug
ninja -C builddir
```
### Generate test coverage report:
```ascii
meson builddir -Db_coverage=true -Dtests=enabled
ninja -C builddir test
ninja -C builddir coverage
```
When BMCWeb starts running, it reads persistent configuration data
(such as UUID and session data) from a local file.  If this is not
usable, it generates a new configuration.

When BMCWeb SSL support is enabled and a usable certificate is not
found, it will generate a self-sign a certificate before launching the
server.  The keys are generated by the `secp384r1` algorithm.  The
certificate
 - is issued by `C=US, O=OpenBMC, CN=testhost`,
 - is valid for 10 years,
 - has a random serial number, and
 - is signed using the `SHA-256` algorithm.

