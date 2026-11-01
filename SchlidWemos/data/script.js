"use strict";

const BALKEN_ANZAHL = 3;
const STATUS_INTERVALL_MS = 2000;

let konfig = null;
let geaendert = false;

const el = {
    luxEin: document.getElementById("luxEin"),
    luxAus: document.getElementById("luxAus"),
    helligkeit: document.getElementById("helligkeit"),
    helligkeitWert: document.getElementById("helligkeitWert"),
    balken: document.getElementById("balken"),
    vorlage: document.getElementById("balkenVorlage"),
    vorschau: document.getElementById("vorschau"),
    speichern: document.getElementById("speichern"),
    meldung: document.getElementById("meldung"),
    statusPunkt: document.getElementById("statusPunkt"),
    statusText: document.getElementById("statusText"),
    schwellenWarnung: document.getElementById("schwellenWarnung"),
};

const zweistellig = (n) => n.toString(16).padStart(2, "0");
const alsHex = (r, g, b) => "#" + zweistellig(r) + zweistellig(g) + zweistellig(b);

function ausHex(hex) {
    const t = /^#([\da-f]{2})([\da-f]{2})([\da-f]{2})$/i.exec(hex);
    return t
        ? { r: parseInt(t[1], 16), g: parseInt(t[2], 16), b: parseInt(t[3], 16) }
        : { r: 0, g: 0, b: 0 };
}

function markiereGeaendert() {
    geaendert = true;
    el.speichern.disabled = false;
    el.meldung.classList.remove("sichtbar");
}

function balkenAufbauen() {
    el.balken.replaceChildren();

    konfig.balken.forEach((bal, i) => {
        const knoten = el.vorlage.content.cloneNode(true);
        const karte = knoten.querySelector(".balken");
        const farbe = knoten.querySelector(".farbe");
        const hex = knoten.querySelector(".hex");
        const prozent = knoten.querySelector(".prozent");
        const prozentWert = knoten.querySelector(".prozentWert");

        knoten.querySelector(".nr").textContent = String(i + 1);
        farbe.value = alsHex(bal.r, bal.g, bal.b);
        farbe.id = "farbe" + i;
        hex.textContent = farbe.value.toUpperCase();
        prozent.value = bal.prozent;
        prozent.id = "prozent" + i;
        prozentWert.htmlFor = prozent.id;
        prozentWert.textContent = bal.prozent + " %";

        farbe.addEventListener("input", () => {
            const c = ausHex(farbe.value);
            bal.r = c.r;
            bal.g = c.g;
            bal.b = c.b;
            hex.textContent = farbe.value.toUpperCase();
            vorschauMalen();
            markiereGeaendert();
        });

        prozent.addEventListener("input", () => {
            bal.prozent = Number(prozent.value);
            prozentWert.textContent = bal.prozent + " %";
            vorschauMalen();
            markiereGeaendert();
        });

        karte.dataset.balken = String(i);
        el.balken.append(knoten);
    });
}

function vorschauMalen() {
    const gesamt = konfig.helligkeit / 100;
    el.vorschau.querySelectorAll(".vorschauBalken").forEach((div) => {
        const bal = konfig.balken[Number(div.dataset.balken)];
        const f = (bal.prozent / 100) * gesamt;
        const r = Math.round(bal.r * f);
        const g = Math.round(bal.g * f);
        const b = Math.round(bal.b * f);
        div.style.background = `rgb(${r} ${g} ${b})`;
    });
}

function schwellenPruefen() {
    el.schwellenWarnung.hidden = konfig.luxAus > konfig.luxEin;
}

function formularFuellen() {
    el.luxEin.value = konfig.luxEin;
    el.luxAus.value = konfig.luxAus;
    el.helligkeit.value = konfig.helligkeit;
    el.helligkeitWert.textContent = konfig.helligkeit + " %";
    balkenAufbauen();
    vorschauMalen();
    schwellenPruefen();
}

function melde(text, fehler) {
    el.meldung.textContent = text;
    el.meldung.classList.toggle("fehler", Boolean(fehler));
    el.meldung.classList.add("sichtbar");
}

async function laden() {
    try {
        const antwort = await fetch("/api/config");
        if (!antwort.ok) {
            throw new Error(antwort.status);
        }
        konfig = await antwort.json();
        formularFuellen();
        el.speichern.disabled = true;
    } catch (e) {
        melde("Konfiguration nicht ladbar", true);
    }
}

async function speichern() {
    el.speichern.disabled = true;
    try {
        const antwort = await fetch("/api/config", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(konfig),
        });
        if (!antwort.ok) {
            throw new Error(antwort.status);
        }
        geaendert = false;
        melde("Gespeichert");
    } catch (e) {
        melde("Nicht gespeichert", true);
        el.speichern.disabled = false;
    }
}

async function statusHolen() {
    try {
        const antwort = await fetch("/api/status");
        const s = await antwort.json();
        el.statusText.textContent = s.lux.toFixed(1) + " lx";
        el.statusPunkt.className = "punkt " + (s.ein ? "an" : "aus");
    } catch (e) {
        el.statusText.textContent = "keine Verbindung";
        el.statusPunkt.className = "punkt";
    }
}

el.luxEin.addEventListener("input", () => {
    konfig.luxEin = Number(el.luxEin.value);
    schwellenPruefen();
    markiereGeaendert();
});

el.luxAus.addEventListener("input", () => {
    konfig.luxAus = Number(el.luxAus.value);
    schwellenPruefen();
    markiereGeaendert();
});

el.helligkeit.addEventListener("input", () => {
    konfig.helligkeit = Number(el.helligkeit.value);
    el.helligkeitWert.textContent = konfig.helligkeit + " %";
    vorschauMalen();
    markiereGeaendert();
});

el.speichern.addEventListener("click", speichern);

window.addEventListener("beforeunload", (ev) => {
    if (geaendert) {
        ev.preventDefault();
        ev.returnValue = "";
    }
});

laden();
statusHolen();
setInterval(statusHolen, STATUS_INTERVALL_MS);
