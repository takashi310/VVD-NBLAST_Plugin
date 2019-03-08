
args = commandArgs(trailingOnly=TRUE)

if (length(args) < 3) stop("At least two argument must be supplied.", call.=FALSE)
infile = args[1]

nlibpath  = args[2]
nlibs = strsplit(nlibpath, ",")
nlibs = nlibs[[1]]

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

if (length(args) >= 6) {
  dbnames = args[6]
} else {
  dbnames = paste(basename(nlibs), collapse=",")
}

if (length(args) >= 7) {
  normalization = args[7]
} else {
  normalization = "mean"
}

cat("Loading NAT...")

if (!require("nat",character.only = TRUE)) {
  install.packages("nat", repos="http://cran.rstudio.com/")
}
if (!require("nat.nblast",character.only = TRUE)) {
  install.packages("nat.nblast", repos="http://cran.rstudio.com/")
}
if (!require("foreach",character.only = TRUE)) {
  install.packages("foreach", repos="http://cran.rstudio.com/")
}
if (!require("doParallel",character.only = TRUE)) {
  install.packages("doParallel", repos="http://cran.rstudio.com/")
}

library(nat.nblast)
library(nat)
library(foreach)
library(parallel)
library(doParallel)

img = NULL
swc = NULL
qdp = NULL
if (grepl("\\.swc$", infile)) {
  cat("Loading swc...\n")
  swc = read.neuron(infile)
  qdp = dotprops(swc, resample=4.0, k=3)
} else {
  cat("Loading images...\n")
  img = read.im3d(infile)
  qdp = dotprops(img, k=3)
}


cat("Initializing threads...\n")
th = parallel::detectCores()-1
if (th < 1) th = 1
cl <- parallel::makeCluster(th, outfile="")


tryCatch({
  
  registerDoParallel(cl)
  cat("Loading NBLAST library into each thread...\n")
  clusterCall(cl, function() suppressMessages(library(nat.nblast)))
  
  sprintf("thread num: %i", th)
  cat("Running NBLAST...\n")
  allres = neuronlist();
  allscr = numeric();
  
  for (i in 1:length(nlibs)) {
    nl <- nlibs[i]
    cat(paste(nl, "\n"))
    dp <- read.neuronlistfh(nl, localdir=dirname(nl))
    
    a <- seq(1, length(dp), by=length(dp)%/%th)
    b <- seq(length(dp)%/%th, length(dp), by=length(dp)%/%th)
    if (length(dp)%%th > 0) {
      b <- c(b, length(dp))
    }
    
    cat("calculating forward scores...\n")
    fwdscores <- foreach(aa = a, bb = b) %dopar% {
      if (aa == 1) {
        nblast(qdp, dp[aa:bb], normalised=T, UseAlpha=T, .progress='text')
      } else {
        nblast(qdp, dp[aa:bb], normalised=T, UseAlpha=T)
      }
    }
    scnames <- list()
    for (j in 1:length(fwdscores)) scnames <- c(scnames, names(fwdscores[[j]]))
    fwdscores <- unlist(fwdscores)
    names(fwdscores) <- scnames
    
    if (normalization == "mean") {
      cat("calculating reverse scores...\n")
      revscores <- foreach(aa = a, bb = b) %dopar% {
        if (aa == 1) {
          nblast(dp[aa:bb], qdp, normalised=T, UseAlpha=T, .progress='text')
        } else {
          nblast(dp[aa:bb], qdp, normalised=T, UseAlpha=T)
        }
      }
      scnames <- list()
      for (j in 1:length(revscores)) scnames <- c(scnames, names(revscores[[j]]))
      revscores <- unlist(revscores)
      names(revscores) <- scnames
      
      scores <- (fwdscores + revscores) / 2
    } else {
      scores <- fwdscores
    }
    
    cat("sorting scores...\n")
    scores <- sort(scores, dec=T)
    
    if (length(scores) <= resultnum) {
      #results <- as.neuronlist(dp[names(scores)])
      slist <- scores
    } else {
      #results <- as.neuronlist(dp[names(scores)[1:resultnum]])
      slist <- scores[1:resultnum]
    }
    
    cat("setting names...\n")
    #for (j in 1:length(results)) {
    #  names(results)[j] <- paste(names(results[j]), as.character(i-1), sep=",")
    #}
    for (j in 1:length(slist)) {
      names(slist)[j] <- paste(names(slist[j]), as.character(i-1), sep=",")
    }
    
    cat("combining lists...\n")
    #allres <- c(allres, results)
    allscr <- c(allscr, slist)
    
    rm(dp)
    gc()
  }
  
  allscr = sort(allscr, dec=T)
  if (length(allscr) <= resultnum) {
    #results = allres[names(allscr)]
    slist = allscr
  } else {
    #results = allres[names(allscr)[1:resultnum]]
    slist = allscr[1:resultnum]
  }
  
  cat("Writing results...\n")
  swczipname = paste(outfname, ".zip", sep="")
  rlistname  = paste(outfname, ".txt", sep="")
  zprojname  = paste(outfname, ".png", sep="")
  
  f = file(file.path(outputdir,rlistname))
  writeLines(c(dbnames, nlibpath), con=f)
  write.table(format(slist, digits=8), append=T, file.path(outputdir,rlistname), sep=",", quote=F, col.names=F, row.names=T)
  #n = names(results)
  #n = gsub(",", " db=", n)
  #write.neurons(results, dir=file.path(outputdir,swczipname), files=n, format='swc', Force=T)
  
  if (!is.null(img)) {
    zproj = projection(img, projfun=max)
    size = dim(zproj)
    png(file.path(outputdir,zprojname), width=size[1], height=size[2])
    par(plt=c(0,1,0,1))
    image(zproj, col = grey(seq(0, 1, length = 256)))
    dev.off()
  } else {
    png(file.path(outputdir,zprojname), width=256, height=256, bg='black')
    plot.new()
    dev.off()
  }
  
}, 
error = function(e) {
  print(e)
  Sys.sleep(5)
  stopCluster(cl)
})

stopCluster(cl)

cat("Done\n")
