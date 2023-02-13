# Flutter JIT embedding


## How to compile flutter code to use this module?

Assuming you have a locally built engine built for "host_debug"
you should have a folder hierarchy such as this:

/flutter
/engine

inside engine folder you should have
/src/output/host_debug_unopt
/src/output/host_debug

These are folders containing the engine compiled with the --debug flag (it means its an engine for running JIT applications in debug mode, not that the engine is built in debug mode!)
the 'unopt' thing means the engine itself is build in debug mode and you can even debug it (since it is locally built)

In you flutter project, you should create an 'app bundle'. I have no idea what exactly is an app_bundle, but in the case of compiling dart code in JIT mode, it will create all the necessary files.

note again the --debug flag means the dart code is JIT (supports hot reload). the --local_engine points to one of the output directories mentioned previously (it assumes engine folder is in the same level as the flutter folder)

```
For windows desktop:
flutter build bundle --debug --local-engine=host_debug_unopt --target-platform=windows-x64 --asset-dir=output_path
```

once completed, the asset file will contain all the nessery files to run the app via the embedder:
- flutter_asset folder
- icuctl.data file

both these paths must be supplied to the embedder as argument.

## How to debug a flutter project?



