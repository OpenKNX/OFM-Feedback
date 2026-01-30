### Buzzer ansteuern über

Die Art, wie der Buzzer extern angesteuert werden kann, kann flexibel bestimmt werden:

* **Ein-/Aus-Signal** - Über ein KO DPT1 wird ein Schaltsignal erwartet. Über einen weiteren Parameter wird die Lautstärke bestimmt.
* **Lautstärke** - Über ein KO DPT5 wird eine Lautstärke vorgegeben: Aus=0, Leise=1, Normal=2, Laut=3
* **Frequenz** - Über ein KO DPT14 wird eine Frequenz vorgegeben, das erlaubt die freie Tonwahl. Frequenz 0 bedeutet aus, der Frequenzbereich liegt zwischen 1500 Hz und 6000 Hz.

