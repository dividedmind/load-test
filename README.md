Load-test
=========

A simple no-overhead multithreaded load tester for Conjur services.

Written in GNU C99, using OpenMP for parallelism and libcurl for Web access.
Compile with `make`.

Usage
-----
The load tests run continuously, reporting current request rate every second.
You can use Ctrl-C to stop it at any time; this causes the tool to stop and 
display a summary before terminating.

Number of threads
-----------------
By default 16 threads are used for sixteen concurrent requests.
You can set a different number of threads with `NUM_THREADS` environment variable.

Load-authn
----------
`load-authn` tests the rate of authentication in conjur-authn service. By default
it connects to http://localhost:5000; use `CONJUR_AUTHN_URL` environment variable
to use a different endpoint.

You need to call `load-authn` with a username and password (or API key) as the 
arguments. This allows to test various conditions, ie. existing accounts, nonexistent
accounts, correct & incorrect passwords, API keys etc.

Note the password is visible on the command line, so don't use sensitive accounts
on multi-user machines; you might want to take care of shell history in any case.

As an alternative, password can be passed in `CONJUR_PASSWD` environment variable.
