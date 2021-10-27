# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.0.2] - 2021-10-24
- Initialize the project - Yu Language (yulang).
### New features
- standard arithmetics and functions.
- local parentheses: `{` `}`.
- multi-line comments `/> ... </`.
- space in `import:` string.
- `exit` command.
- function call without using parameter variables.
- string literal.

### Changes
- in function calls, `=` changes to `:`. Example: `func(a = 5)` to be `func(a : 5)`.
- `print` can be used inside expression. behave like passing objects.
- change `print` to `print(...)` to support multiple variables.

### Fixes
- reusable of variables outside of functions.
- Fixing multi-line comments.

### Bugs
- reusable of variable inside of functions.
- assign variables in function args.