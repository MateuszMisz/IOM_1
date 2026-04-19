# Pseudokody algorytmow - lab1 IMO

---

## Losowe rozwiazanie

```
RANDOM-SOLUTION(n):
  k <- losowa liczba calkowita z [2, n]
  indices <- [0, 1, ..., n-1]
  przetasuj losowo indices
  zwroc indices[0..k-1]
```

---

## Heurystyka najblizszego sasiada (NN)

```
GREEDY-NN(start, useProfit):
  odwiedzone <- {start}
  trasa <- [start]

  dopoki |trasa| < n:
    curr <- trasa.ostatni
    najlepsza_delta <- -nieskonczonosc
    nastepny <- -1

    dla kazdego nieodwiedzonego j:
      delta <- -dist[curr][j]
      jesli useProfit: delta <- delta + profit[j]
      jesli delta > najlepsza_delta:
        najlepsza_delta <- delta
        nastepny <- j

    dodaj nastepny do trasy
    dodaj nastepny do odwiedzonych

  zwroc trasa
  // krawedz zamykajaca: trasa[n-1] -> trasa[0] jest niejawna
```

Wariant NNa: useProfit = false
Wariant NNb: useProfit = true

---

## Metoda rozbudowy cyklu (GC)

```
INSERT-DELTA(a, v, b, useProfit):
  delta <- dist[a][b] - dist[a][v] - dist[v][b]
  jesli useProfit: delta <- delta + profit[v]
  zwroc delta

GREEDY-GC(start, useProfit):
  trasa <- [start]
  // dodaj drugi wierzcholek
  najlepsza_delta <- -nieskonczonosc
  dla kazdego j != start:
    delta <- -dist[start][j]
    jesli useProfit: delta <- delta + profit[j]
    jesli delta > najlepsza_delta:
      najlepsza_delta <- delta
      drugi <- j
  dodaj drugi do trasy

  dopoki |trasa| < n:
    najlepsza_delta <- -nieskonczonosc
    najlepszy_v <- -1
    najlepsza_poz <- -1

    dla kazdego v nie nalezacego do trasy:
      dla kazdej krawedzi (a, b) w cyklu:
        delta <- INSERT-DELTA(a, v, b, useProfit)
        jesli delta > najlepsza_delta:
          najlepsza_delta <- delta
          najlepszy_v <- v
          najlepsza_poz <- pozycja za a

    wstaw najlepszy_v na najlepsza_poz w trasie

  zwroc trasa
```

Wariant GCa: useProfit = false
Wariant GCb: useProfit = true

---

## Heurystyka 2-zalu (RegretGC)

```
REGRET-GC(start, useProfit):
  trasa <- inicjalizacja jak w GREEDY-GC (dwa wierzcholki)

  dopoki |trasa| < n:
    najlepszy_zal <- -nieskonczonosc
    najlepszy_v <- -1
    najlepsza_poz <- -1
    najlepsza_delta1 <- -nieskonczonosc

    dla kazdego v nie nalezacego do trasy:
      delta1 <- -nieskonczonosc
      delta2 <- -nieskonczonosc
      poz1 <- -1

      dla kazdej krawedzi (a, b) w cyklu:
        delta <- INSERT-DELTA(a, v, b, useProfit)
        jesli delta > delta1:
          delta2 <- delta1
          delta1 <- delta
          poz1 <- pozycja za a
        w przeciwnym razie jesli delta > delta2:
          delta2 <- delta

      zal <- delta1 - delta2

      jesli zal > najlepszy_zal:
        najlepszy_zal <- zal
        najlepszy_v <- v
        najlepsza_poz <- poz1
        najlepsza_delta1 <- delta1
      jesli zal == najlepszy_zal i delta1 > najlepsza_delta1:
        najlepszy_v <- v
        najlepsza_poz <- poz1
        najlepsza_delta1 <- delta1

    wstaw najlepszy_v na najlepsza_poz w trasie

  zwroc trasa
```

Wariant RegretGCa: useProfit = false
Wariant RegretGCb: useProfit = true

---

## Wazona heurystyka 2-zalu (WRegretGC)

```
WEIGHTED-REGRET-GC(start, useProfit, w_regret=1, w_cost=1):
  trasa <- inicjalizacja jak w GREEDY-GC (dwa wierzcholki)

  dopoki |trasa| < n:
    najlepszy_score <- -nieskonczonosc
    najlepszy_v <- -1
    najlepsza_poz <- -1

    dla kazdego v nie nalezacego do trasy:
      delta1 <- -nieskonczonosc
      delta2 <- -nieskonczonosc
      poz1 <- -1

      dla kazdej krawedzi (a, b) w cyklu:
        delta <- INSERT-DELTA(a, v, b, useProfit)
        jesli delta > delta1:
          delta2 <- delta1
          delta1 <- delta
          poz1 <- pozycja za a
        w przeciwnym razie jesli delta > delta2:
          delta2 <- delta

      score <- w_regret * (delta1 - delta2) + w_cost * delta1

      jesli score > najlepszy_score:
        najlepszy_score <- score
        najlepszy_v <- v
        najlepsza_poz <- poz1

    wstaw najlepszy_v na najlepsza_poz w trasie

  zwroc trasa
  // domyslnie w_regret=1, w_cost=1 => score = 2*delta1 - delta2
```

Wariant WRegretGCa: useProfit = false
Wariant WRegretGCb: useProfit = true

---

## Faza II - usuwanie wierzcholkow

```
PHASE-II(trasa):
  powtarzaj:
    najlepsza_delta <- 0
    najlepszy_idx <- -1

    dla kazdego wierzcholka curr na pozycji i w trasie:
      prev <- trasa[(i - 1) mod |trasa|]
      next <- trasa[(i + 1) mod |trasa|]

      delta <- dist[prev][curr] + dist[curr][next]
              - dist[prev][next] - profit[curr]

      jesli delta > najlepsza_delta:
        najlepsza_delta <- delta
        najlepszy_idx <- i

    jesli najlepszy_idx != -1:
      usun wierzcholek na pozycji najlepszy_idx z trasy
    w przeciwnym razie:
      przerwij

  zwroc trasa
```
