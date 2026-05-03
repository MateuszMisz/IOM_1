# Pseudokody algorytmow - lab4 IMO

---

## MLSL

```
MLSL(instance, move_type, iterations)

  best_tour<-inicjacja pustego wektora
  best_score<- minimalna wartość
  wykonaj iterations razy:
    tour = randomSolution()
    tour = steepestWalkLM(tour)
    score = evaluate(tour)
    jeśli score > best_score:
      best_tour <- tour
      best_score <- score
  zwróc tour
```
---

## ILS
### ILS

```
ILS(instance, move_type, time_limit, moves_in_perturbation):
  start_time <- odczytaj czas
  tour = randomSolution()
  tour = steepestWalkLM(tour)
  score = evaluate(tour)
  best_tour = tour.copy()
  best_score = score
  Dopóki true:
    perturbated_tour = perturbation(tour,moves_in_perturbation)
    perturbated_tour = steepestWalkLM(perturbated_tour)
    perturbated_score = evaluate(perturbated_tour)
    jeśli przekroczono limit czasu:
      przerwij pętlę
    jeśli perturbated_score > score:
      tour = perturbated_tour
      score = perturbated_score
  zwróc tour
```
### perturbacja
```
perturbation(tour,instance, moves_in_perturbation):
  inTour <- [false] *instance.size()
  dla vortex w tour:
    inTour[vortex] = true
  move_type <- wylosuj typ ruchu {dodaj, usun, zamień_krawędzie}
  jeśli move_type == dodaj i tour_size+moves_in_perturbation > instance.size():
    move_type = remove
  jeśli move_type == usun i tour_size - moves_in_perturbation< 2:
    move_type = add

  move = generateRandomMove(tour,inTour,move_type)
  tour.transform(move)
  zwróc tour

```
### generowanie ruchu losowego
```
generateRandomMove(tour,inTour,move_type):
  jeśli move_type == dodaj:
    idx1 <- wylosuj z wolnych wierzchołków
    idx2 <- wylosuj miejsce wstawienia w tour
  jeśli move_type == usun:
    idx1 <- wylosuj wierzchołek z tour
    idx2 = -1 (dla wspolnego formatu zwracania)
  jeśli move_type == zamień_krawędzie:
    idx1 <- wylosuj pierwszy wierzchołek
    idx2 <- wylosuj drugi wierzchołek
  zwróc Move(move_type,idx1,idx2)
```
---

## LNS
### LNS
```
LNS(instance, time_limit, destruction_rate, use_local_search):
  start_time <- odczytaj czas
  tour = randomSolution()
  tour = steepestWalkLM(tour)
  score = evaluate(tour)
  best_score = score
  dopóki true:
    tmp_solution = destroy(tour)
    tmp_solution = repair(tour)
    jeśli use_local_search:
      tmp_solution = steepestWalkLM(tmp_solution)
    tmp_score = evaluate(tmp_solution)
    jeśli przekroczono limit czasu:
      przerwij pętlę
    jeśli tmp_score > score:
      tour = tmp_solution
      score = tmp_score
  zwróc tour
```
### destrukcja
```
destroy(tour, destruction_rate):
  new_tour <- zainicjuj pusty wektor
  is_removed = [false] * tour.size()
  to_remove = round(tour.size() * destruction_rate)
  start_idx = wylosuj startową pozycje w tour
  dla każdego i w [start_idx,start_idx+to_remove):
    is_removed[i mod tour.size()] = true
  dla każdego i w [0,tour.size):
    jeśli is_removed[i]:
      new_tour.append(tour[i])
  zwróc new_tour

```