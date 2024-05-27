# Centro-Controllo-Droni
Un progetto universitario che simula il funzionamento di droni di controllo all'interno di un'area chiusa.
## Note:
- l'id dei droni si puo mettere serial già nell'sql
- lo stato dei droni nell'sql non è coerente con quanto avevamo concordato. 

è vero che starting e returning sono più o meno uguali e tranfering e starting possono essere tranquillamente accorpati, però sql ha "waiting" che non so che significa e non ha né ready ne starting.
- nell'sql manca il tempo di ricarica dei droni 
## to do:
### trigger:
 - nessun drone può volare se la batteria è scarica
 - nessun drone può trovarsi fuori dai limiti dell'area stabilita
 - nessun drone può uscire dal blocco assegnatogli finché è in stato flying [da valutare]
