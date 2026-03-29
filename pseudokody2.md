# Pseudokody algorytmow - lab1 IMO

### losowe błądzenie
```
randomWalk(baseSolution, timeLimit):
    tour <- baseSolution 
    bestTour <- tour
    bestScore <-evaluate(tour)
    scorr <- bestScore
    delta <- 0
    elapsedTime <- 0
    startTime <- getTime()
    while (timeLimit):
        neighbours <- generateNeighbourhood(tour)
        neighbour <- getRandom(neighbours)
        delta <- transformationDelta(neighbour,tour)
        transform(tour,neighbour)
        score += delta;
            if score > bestScore:
                bestTour <- tour
                bestScore <-score
        elapsedTime <- getTime() - startTime  
    return tour      
```

### lokalne przeszukiwanie w wersji stromej
```
steepestWalk(baseSolution):
    improved <- false
    tour <- baseSolution
    bestDelta <- 0
    while improved:
        improved <- false
        neighbours <- generateNeighbourhood(tour)
        for neighbour in neighbourhood:
            delta <- transformationDelta(neighbour,tour)
            if delta > bestDelta:
                bestDelta = delta
                bestNeighbour = neighbour
        if bestDelta > 0:
            transform(tour,neighbour)
            improved <- true
    return tour
```

### lokalne przeszukiwanie w wersji zachłannej
```
    greedyWalk(baseSolution):
        tour <- baseSolution
        score <- evaluate(tour)
        improved <- true
        while improved:
            improved <- false
            neighbours <- generateNeighbourhood(tour)
            shuffle(neighbours)
            for neighbour in neighbours:
                delta <- transformationDela(neighbour,tour)
                if delta > 0:
                    transform(neighbour, tour)
                    score += delta
                    imporved <- true
                    break
        return tour
```
