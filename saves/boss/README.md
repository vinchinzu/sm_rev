# Boss Save Fixtures

Save states in this directory are reusable regression fixtures. Tests may refer
to these paths directly, so update the matching test and documentation if a
fixture is replaced.

- `torizo_pre_fight.sav`: Bomb Torizo room state used to validate the modded
  opening blue-orb preset. The fixture starts before the opening attack settles;
  the regression advances it deterministically to the first scheduled orb wave.

Top-level files in `saves/` such as `save*.sav`, `bug-*.sav`, `.srm`, and
ad hoc named saves are treated as local iteration artifacts unless a test points
at a canonical fixture path under a subdirectory like this one.
