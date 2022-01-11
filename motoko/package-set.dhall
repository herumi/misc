-- The upstream seems outdated, hence we define what we need directly below under additions
-- let upstream = https://github.com/dfinity/vessel-package-set/releases/download/mo-0.6.7-20210818/package-set.dhall sha256:c4bd3b9ffaf6b48d21841545306d9f69b57e79ce3b1ac5e1f63b068ca4f89957

let Package =
    { name : Text, version : Text, repo : Text, dependencies : List Text }

let upstream = [] : List Package

let
  -- This is where you can add your own packages to the package-set
  additions = [
       { name = "iterext"
        , version = "v2.0.0"
        , repo = "https://github.com/timohanke/motoko-iterext.git"
        , dependencies = [ "base" ] : List Text
        }
    ]

let
  -- This is where you can override existing packages in the package-set
  overrides = [
        { name = "base"
        , version = "dfx-0.8.4"
        , repo = "https://github.com/dfinity/motoko-base.git"
        , dependencies = [] : List Text
        }   
    ]

-- in  upstream # additions # overrides
in upstream # additions # overrides
