# Security Policy

## Supported versions

Security fixes are normally prepared for the current development branch and released in the next available patch version.

| Version | Security support |
| --- | --- |
| Latest public release | Supported on a best-effort basis |
| Current `main` branch | Supported for development and validation |
| Older releases | Upgrade required unless a maintainer explicitly states otherwise |
| Unofficial or modified binaries | Not supported |

Users should download release assets only from the official repository:

https://github.com/masarray/arsonkupik-obs-audio-enhancer/releases

## Reporting a vulnerability

Do **not** disclose security-sensitive details in a public GitHub issue, pull request, discussion, screenshot, log, or social post.

Use GitHub private vulnerability reporting when it is available for the repository:

https://github.com/masarray/arsonkupik-obs-audio-enhancer/security/advisories/new

A private report should include:

- affected ArSonKuPik version or commit SHA;
- OBS Studio version;
- operating system;
- installation or build method;
- vulnerability class;
- complete reproduction steps;
- proof of concept, when safe to provide;
- realistic impact;
- affected files or components;
- known mitigations or workarounds;
- whether the issue has been disclosed elsewhere;
- preferred name and attribution for a future advisory, or a request to remain anonymous.

When GitHub private reporting is unavailable, contact the maintainer through the GitHub profile at https://github.com/masarray and request a private reporting channel **without including vulnerability details in the initial public message**.

## Security scope

Examples of reports that are in scope include:

- arbitrary code execution;
- memory corruption with a credible security impact;
- privilege escalation;
- unsafe installer behavior;
- path traversal or writing outside the intended plugin directory;
- command injection in build, packaging, or release scripts;
- malicious archive extraction behavior;
- unauthorized workflow or release modification;
- dependency or artifact substitution with a demonstrated attack path;
- exposure of credentials, tokens, or private data by project-controlled code or automation;
- a reproducible denial of service that crosses a reasonable security boundary.

The following are normally not treated as security vulnerabilities unless an exploit or security boundary is demonstrated:

- crackle, pumping, distortion, clipping, stereo behavior, or other audio-quality problems;
- ordinary crashes without evidence of exploitability;
- high CPU use without a security impact;
- missing features or unsupported OBS versions;
- social-engineering claims unrelated to project-controlled systems;
- vulnerabilities only in an unofficial or modified binary;
- issues already fixed in the latest release;
- general hardening suggestions without a concrete vulnerability.

Use the normal issue forms for non-security reports:

https://github.com/masarray/arsonkupik-obs-audio-enhancer/issues/new/choose

## Coordinated disclosure process

The project follows a best-effort coordinated disclosure process:

1. The maintainer reviews the private report and may request clarification or a smaller proof of concept.
2. The affected code, packaging, workflow, or documentation is investigated.
3. A fix and regression test are prepared when the report is confirmed.
4. Release timing and public disclosure are coordinated with the reporter when practical.
5. A security advisory or release note may be published after users have a reasonable opportunity to update.

Response and fix times depend on severity, reproducibility, maintainer availability, and release complexity. This policy does not promise a specific service-level agreement.

## Disclosure expectations

Reporters are asked to:

- allow reasonable time for investigation and remediation;
- avoid accessing, modifying, or deleting data that is not their own;
- avoid disrupting OBS installations, GitHub infrastructure, release systems, or other users;
- test only against systems and accounts they control or are authorized to assess;
- keep technical details private until disclosure is coordinated;
- provide enough information to reproduce the issue safely.

The project will not intentionally pursue action against good-faith research that follows this policy, stays within authorized systems, avoids privacy violations, and does not cause unnecessary harm. This statement is not legal advice and does not authorize testing against third-party systems.

## Release and installation safety

To reduce supply-chain risk:

- download only from the official GitHub Releases page;
- do not install binaries received through unrelated file-sharing sites or direct messages;
- review the release tag and release notes before installation;
- close OBS before replacing plugin files;
- do not run development scripts from an untrusted fork without reviewing them;
- treat unexpected requests for stream keys, tokens, passwords, or account access as fraudulent.

Release assets may not be code-signed on every platform. The absence of a platform signature must not be interpreted as proof that an unofficial file is safe; verify the repository and release source.

## Sensitive information

Never include the following in a public report:

- stream keys;
- API tokens;
- GitHub tokens;
- private repository URLs;
- account credentials;
- personal addresses or phone numbers;
- private file paths when they reveal identity;
- unpublished exploit details.

Redact logs and screenshots before sharing them.

## Security acknowledgements

Confirmed reporters may be credited in a release note or advisory with their permission. The project does not currently operate a paid bug-bounty program.
