{-# LANGUAGE OverloadedStrings #-}

-- | CLI for running Haskell physics predictions.
--
-- Usage:
--   echo '{"state": {...}, "inputs": [...]}' | sm-predict
--   sm-predict --tape inputs.json --output predictions.json
module Main (main) where

import Data.Aeson (FromJSON, ToJSON, eitherDecode, encode, object, withObject, (.:), (.=))
import Data.ByteString.Lazy qualified as BL
import Physics.SM
import System.Environment (getArgs)
import System.Exit (die)
import System.IO (hPutStrLn, stderr)

main :: IO ()
main = do
  args <- getArgs
  case args of
    [] -> runStdin
    ["--tape", inputPath, "--output", outputPath] -> runFile inputPath outputPath
    _ -> usage

usage :: IO ()
usage = die $ unlines
  [ "Usage:"
  , "  sm-predict                             # Read JSON from stdin"
  , "  sm-predict --tape IN --output OUT      # Read/write files"
  , ""
  , "Input JSON format:"
  , "  {\"state\": {...}, \"inputs\": [...]}"
  , ""
  , "Output JSON format:"
  , "  {\"states\": [...]}"
  ]

runStdin :: IO ()
runStdin = do
  input <- BL.getContents
  case eitherDecode input of
    Left err -> die $ "Failed to parse input: " ++ err
    Right req -> do
      let result = processRequest req
      BL.putStr (encode result)

runFile :: FilePath -> FilePath -> IO ()
runFile inputPath outputPath = do
  input <- BL.readFile inputPath
  case eitherDecode input of
    Left err -> die $ "Failed to parse input: " ++ err
    Right req -> do
      let result = processRequest req
      BL.writeFile outputPath (encode result)
      hPutStrLn stderr $ "Wrote " ++ show (length (respStates result)) ++ " states to " ++ outputPath

-- | Request format: initial state + input tape.
data PredictRequest = PredictRequest
  { reqState :: SamusState
  , reqInputs :: [ControllerInput]
  } deriving (Show)

instance FromJSON PredictRequest where
  parseJSON = withObject "PredictRequest" $ \o -> do
    state <- o .: "state"
    inputs <- o .: "inputs"
    return (PredictRequest state inputs)

-- | Response format: resulting states.
newtype PredictResponse = PredictResponse
  { respStates :: [SamusState]
  } deriving (Show)

instance ToJSON PredictResponse where
  toJSON (PredictResponse states) = object ["states" .= states]

-- | Run the Haskell physics kernel.
processRequest :: PredictRequest -> PredictResponse
processRequest (PredictRequest initialSt inputs) =
  let cfg = defaultConfig
      states = runTape cfg initialSt inputs
  in PredictResponse states
