import javax.swing.*;
import java.awt.*;
import java.io.*;

public class CompareSearchGUI extends JFrame {
    private JTextArea outputArea;

    public CompareSearchGUI() {
        super("Search Comparison");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(600, 400);
        JButton button = new JButton("Choose CSV and Compare");
        outputArea = new JTextArea();
        outputArea.setEditable(false);
        button.addActionListener(e -> runComparison());
        add(button, BorderLayout.NORTH);
        add(new JScrollPane(outputArea), BorderLayout.CENTER);
    }

    private void runComparison() {
        JFileChooser chooser = new JFileChooser(".");
        int res = chooser.showOpenDialog(this);
        if (res == JFileChooser.APPROVE_OPTION) {
            File file = chooser.getSelectedFile();
            try {
                Process proc = new ProcessBuilder("./compare_search_methods", file.getAbsolutePath())
                        .redirectErrorStream(true).start();
                BufferedReader reader = new BufferedReader(new InputStreamReader(proc.getInputStream()));
                String line;
                outputArea.setText("");
                while ((line = reader.readLine()) != null) {
                    outputArea.append(line + "\n");
                }
                proc.waitFor();
            } catch (Exception ex) {
                outputArea.setText("Error: " + ex.getMessage());
            }
        }
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new CompareSearchGUI().setVisible(true));
    }
}
