library(ggplot2)
library(scales) 
library(grid)
library(plyr)
library(stringr)

setwd(".")

nb_files=312

for (i in 1:nb_files){
  path <- "data"
  FILE <- read.table(file.path(path,paste(paste("monitoring",i,sep=''),".csv",sep='')),header = TRUE,sep = ",",stringsAsFactors = FALSE)
  colnames(FILE) <- c("timestamp", "memory")
  name=paste(paste("monitoring",i,sep=''),".jpg",sep='')
  ggplot(data=FILE,aes(x=timestamp/60,y=memory/1000000)) +geom_line()+#+ geom_bar(stat="identity") +
    ggtitle("Memory Footprint") +xlab("Time (min)") + ylab("Used Memory (GB)") + theme(plot.title = element_text(hjust = 0.5))+ scale_fill_discrete(name = "Job") + xlim(0,max(FILE$timestamp)/60+10)
  path <- "plots"
  ggsave(file.path(path,name), width=5.43, height=2.54)
}



