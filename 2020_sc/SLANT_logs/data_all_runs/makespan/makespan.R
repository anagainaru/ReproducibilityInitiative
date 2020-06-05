library(ggplot2)
library(scales) 
library(grid)
library(plyr)
library(stringr)

path <- "."
FILE <- read.table(file.path(path,"makespan.csv"),header = TRUE,sep = ",",stringsAsFactors = FALSE)


# Number of intervals for distribution analysis (constant growth). 
# -> Sanity check: n_distrib should be smaller than quant_threshold, ideally by at least an order of magnitude.
n_distrib = 100
#min(100,quant_threshold%/%10)


distrib <- data.frame(matrix(ncol = 2, nrow = 0))
colnames(distrib) <- c("makespan", "count")
x_min = min(FILE$makespan)
x_max = max(FILE$makespan)
for (i in 1:n_distrib){
	## INTERVAL BOUNDS
	value_min = x_min + (x_max - x_min)*(i-1)/n_distrib
	value_max = x_min + (x_max - x_min)*i/n_distrib + 1
	## COUNTING THE NUMBER OF VALUES INCLUDED IN THIS INTERVAL BOUND
	len = length(subset(FILE, makespan < value_max & makespan >= value_min )$makespan)
	value_avg = (value_min + value_max)/2
	distrib[nrow(distrib)+1, ] <- list(value_avg,len)
}
name="app_profile.jpg"
ggplot(data=distrib,aes(x=makespan/60,y=count)) + geom_bar(stat="identity") +
ggtitle("Application profile") +xlab("Time (sec)") + ylab("Count") + theme(plot.title = element_text(hjust = 0.5))+ scale_fill_discrete(name = "Job") + xlim(2000/60,10000/60)
ggsave(file.path(path,name))


