module Main (main) where

import Test.Tasty (TestTree, defaultMain, testGroup)
import Test.MiniCompare qualified
import Test.Properties qualified
import Test.Segments qualified
import Test.Unit qualified

main :: IO ()
main = defaultMain tests

tests :: TestTree
tests = testGroup "Physics.SM"
  [ Test.Unit.tests
  , Test.Properties.tests
  , Test.Segments.tests
  , Test.MiniCompare.tests
  ]

