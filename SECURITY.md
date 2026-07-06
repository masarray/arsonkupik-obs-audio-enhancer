# Security Policy

## Supported versions

Security fixes are handled on the default branch and latest public release unless otherwise stated.

## Reporting a vulnerability

Please do not open a public issue for security-sensitive problems.

Report security concerns privately by contacting the repository owner through GitHub, or by opening a minimal public issue that does not include exploit details and asking for a private contact path.

Useful information to include:

- OBS Studio version
- Windows version
- Plugin release version or commit SHA
- Steps to reproduce
- Expected behavior
- Actual behavior
- Whether the issue can execute code, read/write files unexpectedly, crash OBS, or expose user data

## Scope

In scope:

- Installer or uninstaller behavior
- Build workflow supply-chain issues
- Unsafe file writes or path handling
- Plugin crashes that can be triggered by crafted input
- Dependency or packaging risks

Out of scope:

- Audio quality preferences
- Preset tuning disagreements
- General OBS Studio security issues unrelated to this plugin

## Responsible disclosure

Please allow reasonable time to investigate and patch before publishing details.
