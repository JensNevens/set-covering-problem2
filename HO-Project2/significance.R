##
##  significance.R
##  HO-Project2
##
##  Created by Jens Nevens on 23/04/16.
##  Copyright © 2016 Jens Nevens. All rights reserved.
##

filepath <- '/Users/Jens/Documents/Jens/School/1MA/Heuristic Optimization/Assignments/Assignment2/HO-Project2/HO-Project2'

best <- read.table(paste(filepath, 'best.txt', sep="/"),
				   header=TRUE,
				   row.names=1)

ACO <- read.table(paste(filepath, 'percentage', 'ACO.txt', sep='/'),
			      header=FALSE,
			      row.names=1,
			      col.names=c('instance', 'percentage'))

GEN <- read.table(paste(filepath, 'percentage', 'GEN.txt', sep='/'),
	              header=FALSE,
	              row.names=1,
	              col.names=c('instance', 'percentage'))

p.val <- wilcox.test(ACO$percentage, GEN$percentage, paired=TRUE)$p.value
p.val