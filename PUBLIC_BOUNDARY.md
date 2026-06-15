# Public Boundary

The public-status repository is deliberately narrow.

## Allowed

- High-level progress summaries.
- Release checklists.
- Public roadmap entries.
- Clean build and usage instructions once available.
- Links to released source snapshots or builds.
- Stage1-selected source, docs, tools, subtitles, config, and mapping tables.

## Not Allowed

- Private development logs.
- Temporary reverse-engineering traces.
- Raw generated files from local experiments.
- Local machine paths unless needed in a release note and scrubbed first.
- Secrets, tokens, credentials, or account-specific configuration.
- Copyrighted assets or binaries without an explicit redistribution decision.
- S0/SS0 content during the current public boundary.
- Stage2 or later stage content during the current public boundary.
- Image files during the current public boundary.

## Source Release Rule

The source release must be curated from a clean tree. It should not be created
by copying the private working directory wholesale.
