Xchange Core staging tree 0.12.2
===============================


https://www.xcgtech.com


What is Xchange?
----------------

Xchange is an experimental new digital currency that enables anonymous, instant
payments to anyone, anywhere in the world. Xchange uses peer-to-peer technology
to operate with no central authority: managing transactions and issuing money
are carried out collectively by the network. Xchange Core is the name of the open
source software which enables the use of this currency.

For more information, as well as an immediately useable, binary version of
the Xchange Core software, see https://www.xcgtech.comm


License
-------

Xchange Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/qa) are installed) with: `qa/pull-tester/rpc-tests.py`

The Travis CI system makes sure that every pull request is built for Windows
and Linux, OS X, and that unit and sanity tests are automatically run.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

