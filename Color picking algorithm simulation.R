r <- rep(NA_integer_, 100000) 
g <- rep(NA_integer_, 100000) 
b <- rep(NA_integer_, 100000) 
total <- rep(NA_integer_, 100000) 
random_vec <- rep(NA_integer_, 100000)


for (i in 1:100000){
  random <- sample(1:3, 1, replace = TRUE)
  
  if(random == 1){
    r[i] <- sample(0:255, 1, replace = TRUE)
    temp <- 255-r[i]
    g[i] <- sample(0:temp, 1, replace = TRUE)
    b[i] <- 255 - (r[i]+g[i])
  }
  
  if(random == 2){
    g[i] <- sample(0:255, 1, replace = TRUE)
    temp <- 255-g[i]
    b[i] <- sample(0:temp, 1, replace = TRUE)
    r[i] <- 255 - (b[i]+g[i])
  }
  
  if(random == 3){
    b[i] <- sample(0:255, 1, replace = TRUE)
    temp <- 255-b[i]
    r[i] <- sample(0:temp, 1, replace = TRUE)
    g[i] <- 255 - (r[i]+b[i])
  }
    
  total[i] <- r[i]+g[i]+b[i]
  random_vec[i] <- random
  
}

hist(random_vec, breaks = 5000)
hist(r, breaks = 5000)
hist(g, breaks = 5000)
hist(b, breaks = 5000)
