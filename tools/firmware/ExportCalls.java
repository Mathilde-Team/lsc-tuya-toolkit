// Exporte le graphe d'appels (TSV : appelant, type, appelé) vers le fichier donné en argument.
// type = CALL (appel direct) ou PTR (adresse de fonction stockée : callback / appel indirect).
// @category Firmware
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.RefType;
import java.io.PrintWriter;
import java.util.LinkedHashSet;
import java.util.Set;

public class ExportCalls extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] a = getScriptArgs();
        PrintWriter w = new PrintWriter(a[0]);
        Set<String> seen = new LinkedHashSet<>();
        FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
        while (it.hasNext()) {
            Function f = it.next();
            for (Address ad : f.getBody().getAddresses(true)) {
                for (Reference r : currentProgram.getReferenceManager().getReferencesFrom(ad)) {
                    Function t = currentProgram.getFunctionManager().getFunctionAt(r.getToAddress());
                    if (t == null || t == f) {
                        continue;
                    }
                    RefType rt = r.getReferenceType();
                    String kind = rt.isCall() ? "CALL" : (rt.isData() ? "PTR" : "JMP");
                    seen.add(f.getName() + "\t" + kind + "\t" + t.getName());
                }
            }
        }
        for (String s : seen) {
            w.println(s);
        }
        w.close();
        println("edges: " + seen.size());
    }
}
