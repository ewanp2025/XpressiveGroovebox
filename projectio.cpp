#include "projectio.h"
#include <QFile>
#include <QTextStream>
#include <QtXml/QDomDocument>

bool ProjectIO::saveMmp(const QString& filePath, int bpm, const ChannelConfig channels[6], const bool drumSteps[6][16], const int arrangerState[4][8], const MelodicConfig melodic[3]) {
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement root = doc.createElement("lmms-project");
    root.setAttribute("type", "song");
    root.setAttribute("version", "20");
    doc.appendChild(root);

    QDomElement head = doc.createElement("head");
    head.setAttribute("bpm", QString::number(bpm));
    root.appendChild(head);

    QDomElement song = doc.createElement("song");
    root.appendChild(song);

    QDomElement tc = doc.createElement("trackcontainer");
    tc.setAttribute("type", "song");
    song.appendChild(tc);

    QDomElement bbMaster = doc.createElement("track");
    bbMaster.setAttribute("name", "Beat/Bassline 0");
    bbMaster.setAttribute("type", "1");
    tc.appendChild(bbMaster);

    QDomElement bbtc = doc.createElement("trackcontainer");
    bbtc.setAttribute("type", "bbtrackcontainer");

    QDomElement bbTrack = doc.createElement("bbtrack");
    bbTrack.appendChild(bbtc);
    bbMaster.appendChild(bbTrack);


    const int ticksPerStep = 12;


    for (int r = 0; r < 6; ++r) {
        const ChannelConfig& ch = channels[r];

        QDomElement t = doc.createElement("track");
        t.setAttribute("name", ch.name);
        t.setAttribute("type", "0");
        bbtc.appendChild(t);

        QDomElement it = doc.createElement("instrumenttrack");
        it.setAttribute("vol", QString::number(int(ch.vol * 100)));
        it.setAttribute("pan", "0");
        t.appendChild(it);

        QDomElement inst = doc.createElement("instrument");
        inst.setAttribute("name", "xpressive");

        QDomElement xp = doc.createElement("xpressive");
        xp.setAttribute("W1", ch.w1);
        xp.setAttribute("W2", "");
        xp.setAttribute("W3", "");
        xp.setAttribute("O1", ch.o1);
        xp.setAttribute("O2", "");

        xp.setAttribute("W1sample", "AA=="); xp.setAttribute("W2sample", "AA=="); xp.setAttribute("W3sample", "AA==");
        xp.setAttribute("smoothW1", "0"); xp.setAttribute("smoothW2", "0"); xp.setAttribute("smoothW3", "0");
        xp.setAttribute("interpolateW1", "0"); xp.setAttribute("interpolateW2", "0"); xp.setAttribute("interpolateW3", "0");
        xp.setAttribute("A1", "1"); xp.setAttribute("A2", "1"); xp.setAttribute("A3", "1");
        xp.setAttribute("PAN1", "0"); xp.setAttribute("PAN2", "-1"); xp.setAttribute("RELTRANS", "50"); xp.setAttribute("version", "0.1");

        inst.appendChild(xp);
        it.appendChild(inst);

        QDomElement eldata = doc.createElement("eldata");

        QDomElement elvol = doc.createElement("elvol");
        elvol.setAttribute("att", QString::number(ch.attack));
        elvol.setAttribute("dec", QString::number(ch.decay));
        elvol.setAttribute("sustain", QString::number(ch.sustain));
        elvol.setAttribute("rel", QString::number(ch.release));
        elvol.setAttribute("amt", "1");
        eldata.appendChild(elvol);

        if (ch.filter.enabled) {
            QDomElement elcut = doc.createElement("elcut");
            elcut.setAttribute("att", QString::number(ch.filter.cutA));
            elcut.setAttribute("dec", QString::number(ch.filter.cutD));
            elcut.setAttribute("sustain", QString::number(ch.filter.cutS));
            elcut.setAttribute("rel", QString::number(ch.filter.cutR));
            elcut.setAttribute("amt", "1");
            eldata.appendChild(elcut);

            QDomElement elres = doc.createElement("elres");
            elres.setAttribute("att", QString::number(ch.filter.resA));
            elres.setAttribute("dec", QString::number(ch.filter.resD));
            elres.setAttribute("sustain", QString::number(ch.filter.resS));
            elres.setAttribute("rel", QString::number(ch.filter.resR));
            elres.setAttribute("amt", "1");
            eldata.appendChild(elres);

            QDomElement filterNode = doc.createElement("filter");
            filterNode.setAttribute("type", QString::number(ch.filter.type));
            filterNode.setAttribute("enabled", "1");
            filterNode.setAttribute("cutoff", QString::number(ch.filter.cutBase));
            filterNode.setAttribute("q", QString::number(ch.filter.resBase));
            it.appendChild(filterNode);
        }

        it.appendChild(eldata);

        QDomElement pat = doc.createElement("pattern");
        pat.setAttribute("type", "0");
        pat.setAttribute("pos", "0");
        pat.setAttribute("steps", "16");
        t.appendChild(pat);

        for (int c = 0; c < 16; ++c) {
            if (drumSteps[r][c]) {
                QDomElement n = doc.createElement("note");
                n.setAttribute("pos", QString::number(c * ticksPerStep));
                n.setAttribute("len", QString::number(ticksPerStep));
                n.setAttribute("key", "60");
                n.setAttribute("vol", QString::number(int(ch.vol * 100)));
                pat.appendChild(n);
            }
        }
    }

    for (int b = 0; b < 8; ++b) {
        if (arrangerState[0][b] == 1) {
            QDomElement bbtco = doc.createElement("bbtco");
            bbtco.setAttribute("pos", QString::number(b * 16 * ticksPerStep));
            bbtco.setAttribute("len", "192"); // 16 * 12 = 192
            bbMaster.appendChild(bbtco);
        }
    }


    for (int m = 0; m < 3; ++m) {
        const MelodicConfig& ch = melodic[m];

        QDomElement t = doc.createElement("track");
        t.setAttribute("name", ch.name);
        t.setAttribute("type", "0");
        tc.appendChild(t);

        QDomElement it = doc.createElement("instrumenttrack");
        it.setAttribute("vol", QString::number(int(ch.vol * 100)));
        it.setAttribute("pan", "0");
        t.appendChild(it);

        QDomElement inst = doc.createElement("instrument");
        inst.setAttribute("name", "xpressive");

        QDomElement xp = doc.createElement("xpressive");
        xp.setAttribute("W1", ch.w1);
        xp.setAttribute("W2", ch.w2);
        xp.setAttribute("W3", ch.w3);
        xp.setAttribute("O1", ch.o1);
        xp.setAttribute("O2", ch.o2);

        xp.setAttribute("W1sample", "AA=="); xp.setAttribute("W2sample", "AA=="); xp.setAttribute("W3sample", "AA==");
        xp.setAttribute("smoothW1", "0"); xp.setAttribute("smoothW2", "0"); xp.setAttribute("smoothW3", "0");
        xp.setAttribute("interpolateW1", "0"); xp.setAttribute("interpolateW2", "0"); xp.setAttribute("interpolateW3", "0");
        xp.setAttribute("A1", "1"); xp.setAttribute("A2", "1"); xp.setAttribute("A3", "1");
        xp.setAttribute("PAN1", "0"); xp.setAttribute("PAN2", "-1"); xp.setAttribute("RELTRANS", "50"); xp.setAttribute("version", "0.1");

        inst.appendChild(xp);
        it.appendChild(inst);

        QDomElement eldata = doc.createElement("eldata");

        QDomElement elvol = doc.createElement("elvol");
        elvol.setAttribute("att", QString::number(ch.attack));
        elvol.setAttribute("dec", QString::number(ch.decay));
        elvol.setAttribute("sustain", QString::number(ch.sustain));
        elvol.setAttribute("rel", QString::number(ch.release));
        elvol.setAttribute("amt", ch.useAdsr ? "1" : "0");
        eldata.appendChild(elvol);

        if (ch.filter.enabled) {
            QDomElement elcut = doc.createElement("elcut");
            elcut.setAttribute("att", QString::number(ch.filter.cutA));
            elcut.setAttribute("dec", QString::number(ch.filter.cutD));
            elcut.setAttribute("sustain", QString::number(ch.filter.cutS));
            elcut.setAttribute("rel", QString::number(ch.filter.cutR));
            elcut.setAttribute("amt", "1");
            eldata.appendChild(elcut);

            QDomElement elres = doc.createElement("elres");
            elres.setAttribute("att", QString::number(ch.filter.resA));
            elres.setAttribute("dec", QString::number(ch.filter.resD));
            elres.setAttribute("sustain", QString::number(ch.filter.resS));
            elres.setAttribute("rel", QString::number(ch.filter.resR));
            elres.setAttribute("amt", "1");
            eldata.appendChild(elres);

            QDomElement filterNode = doc.createElement("filter");
            filterNode.setAttribute("type", QString::number(ch.filter.type));
            filterNode.setAttribute("enabled", "1");
            filterNode.setAttribute("cutoff", QString::number(ch.filter.cutBase));
            filterNode.setAttribute("q", QString::number(ch.filter.resBase));
            it.appendChild(filterNode);
        }

        it.appendChild(eldata);

        for (int b = 0; b < 8; ++b) {
            int pIdx = arrangerState[m + 1][b];
            if (pIdx == 0) continue;
            int p = pIdx - 1;

            bool hasNotes = false;
            for (int k = 0; k < 96; ++k) {
                for (int c = 0; c < 16; ++c) {
                    if (ch.grid[p][k][c].active) hasNotes = true;
                }
            }
            if (!hasNotes) continue;

            QDomElement pat = doc.createElement("pattern");
            pat.setAttribute("type", "1");
            pat.setAttribute("pos", QString::number(b * 16 * ticksPerStep));
            pat.setAttribute("steps", "16");
            pat.setAttribute("len", QString::number(16 * ticksPerStep));
            t.appendChild(pat);

            for (int k = 0; k < 96; ++k) {
                for (int c = 0; c < 16; ++c) {
                    if (ch.grid[p][k][c].active) {
                        QDomElement n = doc.createElement("note");
                        n.setAttribute("pos", QString::number(c * ticksPerStep));
                        n.setAttribute("len", QString::number(ch.grid[p][k][c].length * ticksPerStep));
                        n.setAttribute("key", QString::number(107 - k));
                        n.setAttribute("vol", QString::number(int(ch.vol * 100)));
                        pat.appendChild(n);
                    }
                }
            }
        }
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << doc.toString(4);
    file.close();
    return true;
}

bool ProjectIO::loadMmp(const QString& filePath, int& bpm, ChannelConfig channels[6], bool drumSteps[6][16], int arrangerState[4][8], MelodicConfig melodic[3]) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QDomDocument doc;
    if (!doc.setContent(&file)) return false;

    QDomElement root = doc.documentElement();
    bpm = root.firstChildElement("head").attribute("bpm", "120").toInt();

    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 16; ++c) {
            drumSteps[r][c] = false;
        }
    }

    for (int t = 0; t < 4; ++t) {
        for (int b = 0; b < 8; ++b) {
            arrangerState[t][b] = 0;
            if (t > 0) {
                for (int k = 0; k < 96; ++k) {
                    for (int c = 0; c < 16; ++c) {
                        melodic[t - 1].grid[b][k][c].active = false;
                    }
                }
            }
        }
    }

    QDomNode n = root.firstChildElement("song").firstChildElement("trackcontainer").firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull() && e.tagName() == "track") {
            if (e.attribute("type") == "1") {
                QDomNodeList bbtcos = e.elementsByTagName("bbtco");
                for (int i = 0; i < bbtcos.size(); ++i) {

                    int bar = bbtcos.at(i).toElement().attribute("pos").toInt() / 192;
                    if (bar >= 0 && bar < 8) arrangerState[0][bar] = 1;
                }

                QDomNodeList bbTracks = e.firstChildElement("bbtrack").firstChildElement("trackcontainer").elementsByTagName("track");
                for (int i = 0; i < bbTracks.size(); ++i) {
                    QString name = bbTracks.at(i).toElement().attribute("name");
                    int dIdx = -1;
                    for (int r = 0; r < 6; ++r) {
                        if (channels[r].name == name) dIdx = r;
                    }

                    if (dIdx != -1) {
                        QDomElement xp = bbTracks.at(i).toElement().firstChildElement("instrumenttrack").firstChildElement("instrument").firstChildElement("xpressive");
                        if (!xp.isNull()) {
                            channels[dIdx].w1 = xp.attribute("W1");
                            channels[dIdx].o1 = xp.attribute("O1");
                        }

                        QDomElement elvol = bbTracks.at(i).toElement().firstChildElement("instrumenttrack").firstChildElement("eldata").firstChildElement("elvol");
                        if (!elvol.isNull()) {
                            channels[dIdx].attack = elvol.attribute("att").toDouble();
                            channels[dIdx].decay = elvol.attribute("dec").toDouble();
                            channels[dIdx].sustain = elvol.attribute("sustain").toDouble();
                            channels[dIdx].release = elvol.attribute("rel").toDouble();
                        }

                        QDomNodeList notes = bbTracks.at(i).toElement().firstChildElement("pattern").elementsByTagName("note");
                        for (int nIdx = 0; nIdx < notes.size(); ++nIdx) {

                            int step = notes.at(nIdx).toElement().attribute("pos").toInt() / 12;
                            if (step >= 0 && step < 16) drumSteps[dIdx][step] = true;
                        }
                    }
                }
            } else if (e.attribute("type") == "0") {
                QString name = e.attribute("name");
                int mIdx = -1;
                for (int m = 0; m < 3; ++m) {
                    if (melodic[m].name == name) mIdx = m;
                }

                if (mIdx != -1) {
                    QDomElement xp = e.firstChildElement("instrumenttrack").firstChildElement("instrument").firstChildElement("xpressive");
                    if (!xp.isNull()) {
                        melodic[mIdx].w1 = xp.attribute("W1");
                        melodic[mIdx].w2 = xp.attribute("W2");
                        melodic[mIdx].o1 = xp.attribute("O1");
                        melodic[mIdx].o2 = xp.attribute("O2");
                    }

                    QDomElement elvol = e.firstChildElement("instrumenttrack").firstChildElement("eldata").firstChildElement("elvol");
                    if (!elvol.isNull()) {
                        melodic[mIdx].attack = elvol.attribute("att").toDouble();
                        melodic[mIdx].decay = elvol.attribute("dec").toDouble();
                        melodic[mIdx].sustain = elvol.attribute("sustain").toDouble();
                        melodic[mIdx].release = elvol.attribute("rel").toDouble();
                        if (elvol.hasAttribute("amt")) {
                            melodic[mIdx].useAdsr = (elvol.attribute("amt").toInt() > 0);
                        }
                    }

                    QDomNodeList patterns = e.elementsByTagName("pattern");
                    for (int p = 0; p < patterns.size(); ++p) {

                        int bar = patterns.at(p).toElement().attribute("pos").toInt() / 192;
                        if (bar >= 0 && bar < 8) {
                            arrangerState[mIdx + 1][bar] = bar + 1;
                            QDomNodeList notes = patterns.at(p).toElement().elementsByTagName("note");
                            for (int nIdx = 0; nIdx < notes.size(); ++nIdx) {

                                int step = notes.at(nIdx).toElement().attribute("pos").toInt() / 12;
                                int key = 107 - notes.at(nIdx).toElement().attribute("key").toInt();
                                int len = notes.at(nIdx).toElement().attribute("len", "12").toInt() / 12;
                                if (step >= 0 && step < 16 && key >= 0 && key < 96) {
                                    melodic[mIdx].grid[bar][key][step].active = true;
                                    melodic[mIdx].grid[bar][key][step].length = len;
                                }
                            }
                        }
                    }
                }
            }
        }
        n = n.nextSibling();
    }
    return true;
}