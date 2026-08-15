# Sample Tape

`run_right.json` is a leftover Haskell self-output sample (B+Right,
`predictor: "haskell-v1"`). It is **not** an H↔M tape.

Tests no longer file-exist-check this directory. Residual compares live in
`Test.Unit`, `Test.Segments`, and `Test.MiniCompare` (the last one calls
`sm_rev_predict` when present).
