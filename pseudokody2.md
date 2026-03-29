# Pseudokody algorytmow - lab1 IMO

### losowe błądzenie
```
randomWalk(baseSolution, timeLimit,interTourMoveType):
    tour <- baseSolution 
    bestTour <- tour
    bestScore <-evaluate(tour)
    scorr <- bestScore
    delta <- 0
    elapsedTime <- 0
    startTime <- getTime()
    while (timeLimit):
        neighbours <- generateNeighbourhood(tour,interTourMoveType)
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
        neighbours <- generateNeighbourhood(tour,interTourMoveType)
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
    greedyWalk(baseSolution,interTourMoveType):
        tour <- baseSolution
        score <- evaluate(tour)
        improved <- true
        while improved:
            improved <- false
            neighbours <- generateNeighbourhood(tour,interTourMoveType)
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

### generowanie sąsiadów
```
    generateNeighbourhood(tour, interTourMoveType):
        neighbours <- []
        generateAddNeighbourhood(tour, out_nieghbours)
        generateRemoveNeighbourhood(tour,out_nieghbours)
        if interTourMoveType == "swapVertices":
            generateSwapVerticesNeighbourhood(tour,out_nieghbours)
        else if inteTourMoveType == "swapEdges":
            generateSwapEdgesNeighbourhood(tour,out_nieghbours)
        return neighbours  


    generateAddNeighbourhood(tour,out_nieghbours):
        inTour <- [false] of instance size
        for vortex in tour:
            in tour[vortex] = true
        k <-tour.size()
        for v ∈ 0...instanceSize:
            if inTour[v]:
                continue
            for i ∈ 0...k:
                neighbour <-new Neighbour
                neighbour.first <- v
                neighbour.second <- i
                neighbour.type <- "add"
                out_neighbours.push_back(neighbour)
    
    generatreRemoveNeighbourHood(tour, out_neighbours):
        for i ∈ 0...tour.size()-1:
            neighbour <- new Neighbour
            neighbour.first <- v
            neighbour.second <- -1 //to avoid random numbers
            neighbour.type <- "remove"
            out_neighbours.push_back(neighbour)
    
    generateSwapVerticesNeighbourhood(tour,out_nieghbours):
        k <- tour.size()
        for i ∈ 0...k-1:
            for j ∈ i+1...k-1:
                neighbour <- new Neighbour
                neighbour.first <- i
                neighbour.second <- j
                neighbour.type <- "swapVertices"
                out_neighbours.push_back(neighbour)
    
    generateSwapEdgesNeighbourhood(tour,out_neighbours):
    k <- tour.size()
        for i ∈ 0...k-1:
            for j ∈ i+2...k:
                neighbour <- new Neighbour
                neighbour.first <- i
                neighbour.second <- j
                neighbour.type <- "swapVertices"
                out_neighbours.push_back(neighbour)


```
