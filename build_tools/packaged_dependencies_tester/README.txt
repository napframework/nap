This script is intended to do some crude testing on packaged NAP framework releases to detect basic issues
mainly relating to dependencies. The script should be run on each packaged framework release (on all 
platforms) before the release is published.

During the testing each demo is configured, built, packaged, run from normal build output and run from the 
packaged project. Napkin is run from normal build output and a packaged project. A project is created from 
the template and then built, packaged, run from normal build output and run from the packaged project.

On macOS and Linux the packaged NAP framework directory and Qt library directory are renamed before the 
packaged projects are run in an attempt to avoid dependency issues being hidden by libraries sourced from 
those paths. One impact of this is the process running in a slightly strange order. Another impact is that
the test suite should probably be the only (NAP-related) thing running on those systems at that time.

A JSON report is populated to report.json, and on failure STDOUT and STDERR are included in the report.
An option is available to also include the logs into the report on success.

Other command line options are available to control other parts of the behaviour.


Usage:

    Create a framework release then run the script on the extracted directory of the release, eg.:
    1. package -nt
    2. move NAP-0.2.3-Win64.zip ..
    3. cd ..
    4. * extract the ZIP in place *
    5. python packaged_dependencies_tester.py NAP-0.2.3-Win64


Known issues:

    - Strangely some demos (currently videomodulation and vinyl) on Windows and Napkin on macOS don't display
      a window when their STDOUT stream is being captured. However viewing their log they still appear to 
      running sufficiently for the purpose of the dependencies test.
    - Shortcomings in ensuring we're not filling dependencies with libraries from unexpected locations on 
      *nix (see below)


Future potential changes:

    - ** Considerations for dependencies getting pulled in from the thirdparty directory used to build the 
      NAP framework release on *nix **
    - Adding options for report output location and working directory for packaged projects (if tying into 
      automated testing)
    - Detect system-wide Qt installation (eg. Homebrew, apt) on *nix and warn (as this could hide missing 
      dependencies)
    - Allow it to operate on a zipped framework release
    - Make a set of test projects which include a minimum set of NAP modules, identifying cases where some 
      modules may unintentionally only work when combined with others. This would require a separate platform
      build that includes these projects though, which maybe could involve the script itself wrapping a 
      packaging process.