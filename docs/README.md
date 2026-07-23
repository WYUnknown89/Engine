# Documentation

## Canonical Documents

- [Master implementation specification](Master_Implementation_Specification.md)
- [Secondary specification PDF](ARPG_Engine_Master_Implementation_Specification_v1.1.pdf)
- [Pre-implementation audit](Pre_Implementation_Audit.md)
- [Project rules](PROJECT_RULES.md)
- [Architecture decisions](Architecture_Decisions.md)
- [Roadmap](ROADMAP.md)
- [M0 validation status](M0_Validation.md)
- [M1 validation status](M1_Validation.md)
- [M2 validation status](M2_Validation.md)
- [M3 validation status](M3_Validation.md)
- [Changelog](CHANGELOG.md)

## ADRs

See `docs/ADR`.

## Secondary PDF

The Markdown specification is canonical. On the current Linux host, regenerate
the secondary PDF with:

```bash
output_dir=$(mktemp -d)
libreoffice --headless --convert-to pdf --outdir "$output_dir" \
  docs/Master_Implementation_Specification.md
cp "$output_dir/Master_Implementation_Specification.pdf" \
  docs/ARPG_Engine_Master_Implementation_Specification_v1.1.pdf
```

Validate its visible version with `pdftotext`; PDF metadata never overrides the
Markdown source.
