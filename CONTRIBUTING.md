# Contributing to KScreen

 - [Code of Conduct](#code-of-conduct)
 - [Submission Guideline](#submission-guideline)
 - [Commit Message Guideline](#commit-message-guideline)
 - [Contact](#contact)

## Code of Conduct
The [KDE Community Code of Conduct][kde-coc] is applied.

You can reach out to the [Commmunity Working Group][community-working-group] if you have questions about the Code of Conduct or if you want to get help on solving an issue with another contributor or maintainer.

## Submission Guideline
The project follows the [Frameworks Coding Style][frameworks-style].

All non-trivial patches need to go through [Phabricator reviews][phab-reviews].

Commits are applied directly on top of master or a bug-fix branch and without merge commits. Larger changes must be split up into smaller logical commits each with a separate review. These reviews must be marked with Phabricator's online tools as dependent on each other.

KScreen is released as part of Plasma. See the [Plasma schedule][plasma-schedule] for information on when the next new major version is released from master branch or a minor release with changes from one of the bug-fix branches.

## Commit Message Guideline
Besides the [KDE Commit Policy][commit-policy] the [Conventional Commits 1.0.0][conventional-commits] specification is applied with the following amendments:

* Only the following types are allowed:
  * build: changes to the CMake build system, dependencies or other build-related tooling
  * docs: documentation only changes to overall project or code
  * feat: new feature
  * fix: bug fix
  * perf: performance improvement
  * refactor: rewrite of code logic that neither fixes a bug nor adds a feature
  * style: improvements to code style without logic change
  * test: addition of a new test or correction of an existing one
* Only the following optional scopes are allowed:
  * kcm
  * kded
  * plasmoid
* Angular's [Revert][angular-revert] and [Subject][angular-subject] policies are applied.
* Breaking changes are supposed to be pointed out only in prose in the commit body.
* When a commit closes a bug on [Bugzilla][bugzilla] or when the commit has an associated Phabricator review special keywords must be used in the commit body to link the respective bug or review. See [here][commit-policy-keywords] for more information on these and other possible keywords. [Arcanist][arcanist] should be used for automating usage of the review keyword.

Commits deliberately ignoring this guideline will be reverted.

### Example

    fix(kcm): provide correct return value

    For function exampleFunction the return value was incorrect.
    Instead provide the correct value A by changing B to C.

    BUG: <bug-number>

    Differential Revision: https://phabricator.kde.org/<review-number>

## Contact
Real-time communication about the project happens on the IRC channel `#plasma` on freenode and the bridged Matrix room `#plasma:kde.org`.

Emails about the project can be sent to the [plasma-devel][plasma-devel] mailing list.

[kde-coc]: https://kde.org/code-of-conduct
[community-working-group]: https://ev.kde.org/workinggroups/cwg.php
[frameworks-style]: https://community.kde.org/Policies/Frameworks_Coding_Style
[phab-reviews]: https://phabricator.kde.org/differential
[plasma-schedule]: https://community.kde.org/Schedules/Plasma_5
[commit-policy]: https://community.kde.org/Policies/Commit_Policy
[conventional-commits]: https://www.conventionalcommits.org/en/v1.0.0/#specification
[angular-revert]: https://github.com/angular/angular/blob/3cf2005a936bec2058610b0786dd0671dae3d358/CONTRIBUTING.md#revert
[angular-subject]: https://github.com/angular/angular/blob/3cf2005a936bec2058610b0786dd0671dae3d358/CONTRIBUTING.md#subject
[bugzilla]: https://bugs.kde.org/describecomponents.cgi?product=KScreen
[commit-policy-keywords]: https://community.kde.org/Policies/Commit_Policy#Special_keywords_in_GIT_and_SVN_log_messages
[arcanist]: https://secure.phabricator.com/book/phabricator/article/arcanist
[plasma-devel]: https://mail.kde.org/mailman/listinfo/plasma-devel
