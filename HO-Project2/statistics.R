##
##  lsscp.c
##  HO-Project2
##
##  Created by Jens Nevens on 16/04/16.
##  Copyright © 2016 Jens Nevens. All rights reserved.
##

folder <- '/Users/Jens/Documents/Jens/School/1MA/Heuristic Optimization/Assignments/Assigment2/HO-Project2/HO-Project2'

best <- read.table(paste(filepath, 'best.txt', sep='/'),
				   header=FALSE,
				   row.names=1)

acocost <- read.table(paste(filepath, 'ACO-output', 'aco.txt', sep='/'),
					  header=FALSE,
					  row.names=1)

perct.deviation <- data.frame(row.names=rownames(best),
	                          perct=100*((acocost$V2 - best$V2)/best$V2))

avg.deviation <- mean(perct.deviation$perct)

perct.deviation
avg.deviation