// Exporte functions.tsv (adresse, nom, taille, fichiers sources déduits, nb chaînes) et
// strings.tsv (fonction, chaîne) pour tout le programme. Arguments : dossier de sortie.
// @category Firmware
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.StringDataInstance;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.Reference;
import java.io.PrintWriter;
import java.util.LinkedHashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ExportAll extends GhidraScript {
    private static final Pattern SRC = Pattern.compile("([A-Za-z0-9_]+\\.(?:c|cpp|h))");

    @Override
    public void run() throws Exception {
        String dir = getScriptArgs()[0];
        PrintWriter fw = new PrintWriter(dir + "/functions.tsv");
        PrintWriter sw = new PrintWriter(dir + "/strings.tsv");
        FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
        while (it.hasNext()) {
            Function f = it.next();
            Set<String> strs = new LinkedHashSet<>();
            Set<String> srcs = new LinkedHashSet<>();
            for (Address ad : f.getBody().getAddresses(true)) {
                for (Reference r : currentProgram.getReferenceManager().getReferencesFrom(ad)) {
                    Data d = currentProgram.getListing().getDataAt(r.getToAddress());
                    if (d == null || !d.hasStringValue()) {
                        continue;
                    }
                    String s = d.getDefaultValueRepresentation();
                    if (s.length() > 200) {
                        s = s.substring(0, 200);
                    }
                    strs.add(s);
                    Matcher m = SRC.matcher(s);
                    while (m.find()) {
                        srcs.add(m.group(1));
                    }
                }
            }
            fw.println(f.getEntryPoint() + "\t" + f.getName() + "\t" + f.getBody().getNumAddresses() + "\t"
                    + String.join(",", srcs) + "\t" + strs.size());
            for (String s : strs) {
                sw.println(f.getName() + "\t" + s.replace('\t', ' ').replace('\n', ' '));
            }
        }
        fw.close();
        sw.close();
        println("functions exported");
    }
}
