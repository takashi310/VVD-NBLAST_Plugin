
args = commandArgs(trailingOnly=TRUE)

if (length(args) < 3) stop("At least two argument must be supplied.", call.=FALSE)
imagefile = args[1]
nlibpath  = args[2]

if (length(args) >= 3) {
  outfname = args[3]
} else {
  outfname = paste(basename(imagefile), ".nblust", sep="")
}

if (length(args) >= 4) {
	outputdir = args[4]
} else {
	outputdir = dirname(imagefile)
}
if (!dir.exists(outputdir)) {
    dir.create(outputdir, FALSE)
}

if (length(args) >= 5) {
	resultnum = strtoi(args[5])
	if (is.na(resultnum)) resultnum = 10
} else {
  resultnum = 10
}

cat("Loading NAT...")

if (!require("nat",character.only = TRUE)) {
  if (!requireNamespace("devtools")) install.packages("devtools")
  devtools::install_github("jefferis/nat")
}
if (!require("nat.nblast",character.only = TRUE)) {
  devtools::install_github("jefferislab/nat.nblast")
}

library(nat.nblast)
library(nat)
library(foreach)
library(parallel)
library(doParallel)

cat("Loading neuron libraries...\n")
dp = read.neuronlistfh(nlibpath, localdir=dirname(nlibpath))

cat("Loading images...\n")
img = read.im3d(imagefile)

cat("Running NBLAST...\n")

cl <- parallel::makeCluster(parallel::detectCores()-1)
registerDoParallel(cl)

scores = nblast(dotprops(img), dp, normalised=T, UseAlpha=T, .parallel=T, .progress='text')
scores = sort(scores, dec=T)

if (length(scores) <= resultnum) {
  results = dp[names(scores)]
  slist = scores
} else {
  results = dp[names(scores)[1:resultnum]]
  slist = scores[1:resultnum]
}

cat("Writing results...\n")
swczipname = paste(outfname, ".zip", sep="")
rlistname  = paste(outfname, ".txt", sep="")
zprojname  = paste(outfname, ".png", sep="")

write.neurons(results, dir=file.path(outputdir,swczipname), files=names(results), format='swc', Force=T)
write.table(format(slist, digits=15), file.path(outputdir,rlistname), sep=",", quote=F, col.names=F, row.names=T)

zproj = projection(img, projfun=max)
size = dim(zproj)
png(file.path(outputdir,zprojname), size[1], size[2])
par(plt=c(0,1,0,1))
image(zproj, col = grey(seq(0, 1, length = 256)))
dev.off()

stopCluster(cl)

cat("Done\n")
