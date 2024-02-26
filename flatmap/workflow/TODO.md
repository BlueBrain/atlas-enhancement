# TODO
## Good ideas
+ Use JSON user configuration files, parsed with `jq`
+ Use NRRD input files in flatpath, avoid conversion to .bin.gz
+ Output streamlines as separate step, then use them for intersections, thickness, depth, etc.
+ Move from text outputs to binary outputs
+ Move from text utils to binary-handling utils
+ Add plotting distribution of nearest-approximation error
+ Consolidate metrics into a single python package
+ Fix direction of relative depth field and orientation vectors.
## Done
+ Instead of relative links joining stages, create links using known global file names across stages, this way if the file names change, the links are updated automatically
+ Add export target to move all outputs to a folder
