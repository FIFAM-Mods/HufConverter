import sys
import os
import subprocess
from PyQt5.QtWidgets import (
    QApplication, QWidget, QLabel, QLineEdit, QPushButton, QComboBox, QStyle,
    QFileDialog, QCheckBox, QVBoxLayout, QHBoxLayout, QMessageBox, QGridLayout
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont, QIcon

class HufConverterGUI(QWidget):
    def __init__(self):
        super().__init__()
        style = self.style()
        icon = style.standardIcon(QStyle.SP_FileDialogListView)
        self.setWindowIcon(icon)
        self.setWindowTitle("HufConverterGUI")
        self.setFixedSize(600, 300)
        self.init_ui()

    def init_ui(self):
        layout = QGridLayout()

        font = QFont()
        font.setPointSize(10)
        largeFont = QFont()
        largeFont.setPointSize(12)

        # Game selection
        game_label = QLabel("Game:")
        game_label.setFont(font)
        layout.addWidget(game_label, 0, 0)
        self.gameCombo = QComboBox()
        self.gameCombo.setFont(font)
        self.gameCombo.addItems([
            "FIFA Manager 09 - FIFA Manager 14",
            "FIFA Manager 06 - FIFA Manager 08",
            "Total Club Manager 2005"
        ])
        layout.addWidget(self.gameCombo, 0, 1, 1, 2)

        # Input File
        input_label = QLabel("Input File:")
        input_label.setFont(font)
        layout.addWidget(input_label, 1, 0)
        self.inputFileEdit = QLineEdit()
        self.inputFileEdit.setFont(font)
        self.inputFileEdit.textChanged.connect(self.detect_input_format)
        layout.addWidget(self.inputFileEdit, 1, 1)
        browseBtn = QPushButton("Browse")
        browseBtn.setFont(font)
        browseBtn.setFixedHeight(28)
        browseBtn.clicked.connect(self.browse_input_file)
        layout.addWidget(browseBtn, 1, 2)

        # Input Format
        input_format_label = QLabel("Input File Format:")
        input_format_label.setFont(font)
        layout.addWidget(input_format_label, 2, 0)
        self.inputFormatCombo = QComboBox()
        self.inputFormatCombo.setFont(font)
        self.inputFormats = [
            "Translation File (.HUF)",
            "Excel Workbook (.XLSX)",
            "Unicode Text (.TXT)",
            "Comma Separated (.CSV)",
            "Semicolon Separated (.CSV)",
            "Tab Separated (.TSV)",
            "Custom Translation (.TR)"
        ]
        self.inputFormatCombo.addItems(self.inputFormats)
        self.inputFormatCombo.currentTextChanged.connect(self.update_output_format)
        layout.addWidget(self.inputFormatCombo, 2, 1, 1, 2)

        # Output Format
        output_format_label = QLabel("Output File Format:")
        output_format_label.setFont(font)
        layout.addWidget(output_format_label, 3, 0)
        self.outputFormatCombo = QComboBox()
        self.outputFormatCombo.setFont(font)
        layout.addWidget(self.outputFormatCombo, 3, 1, 1, 2)

        # Output File (Optional)
        output_file_label = QLabel("Output File (Optional):")
        output_file_label.setFont(font)
        layout.addWidget(output_file_label, 4, 0)
        self.outputFileEdit = QLineEdit()
        self.outputFileEdit.setFont(font)
        layout.addWidget(self.outputFileEdit, 4, 1, 1, 2)

        # Match Key Names
        self.matchKeysCheck = QCheckBox("Match Key Names")
        self.matchKeysCheck.setFont(font)
        self.matchKeysCheck.setChecked(True)
        layout.addWidget(self.matchKeysCheck, 5, 0, 1, 3)

        # Convert Button
        self.convertBtn = QPushButton("Convert")
        self.convertBtn.setFont(largeFont)
        self.convertBtn.setFixedHeight(40)
        self.convertBtn.clicked.connect(self.convert_file)
        layout.addWidget(self.convertBtn, 6, 0, 1, 3)

        self.setLayout(layout)
        self.inputFormatCombo.setCurrentIndex(0)
        self.update_output_format()

    def browse_input_file(self):
        fileName, _ = QFileDialog.getOpenFileName(self, "Select Input File")
        if fileName:
            self.inputFileEdit.setText(fileName)

    def detect_input_format(self):
        path = self.inputFileEdit.text().lower()
        ext_map = {
            ".huf": "Translation File (.HUF)",
            ".xlsx": "Excel Workbook (.XLSX)",
            ".txt": "Unicode Text (.TXT)",
            ".csv": "Comma Separated (.CSV)",  # default to comma
            ".tsv": "Tab Separated (.TSV)",
            ".tr": "Custom Translation (.TR)"
        }
        for ext, name in ext_map.items():
            if path.endswith(ext):
                self.inputFormatCombo.setCurrentText(name)
                break

    def update_output_format(self):
        input_text = self.inputFormatCombo.currentText()
        self.outputFormatCombo.blockSignals(True)
        self.outputFormatCombo.clear()

        if input_text == "Translation File (.HUF)":
            self.outputFormatCombo.addItems([
                "Excel Workbook (.XLSX)",
                "Unicode Text (.TXT) (UTF-16 LE BOM)",
                "Comma Separated (.CSV) (UTF-8 BOM)",
                "Semicolon Separated (.CSV) (UTF-8 BOM)",
                "Tab Separated (.TSV) (UTF-8 BOM)",
                "Custom Translation (.TR) (UTF-8 BOM)"
            ])
        else:
            self.outputFormatCombo.addItem("Translation File (.HUF)")

        self.outputFormatCombo.blockSignals(False)

    def convert_file(self):
        input_path = self.inputFileEdit.text().strip()
        output_path = self.outputFileEdit.text().strip()

        if not os.path.isfile(input_path):
            QMessageBox.critical(self, "Error", "Invalid input file.")
            return

        if output_path:
            output_dir = os.path.dirname(output_path)
            if output_dir and not os.path.isdir(output_dir):
                QMessageBox.critical(self, "Error", "Output directory does not exist.")
                return

        exe_path = os.path.join(os.path.dirname(__file__), "HufConverter.exe")
        if not os.path.isfile(exe_path):
            QMessageBox.critical(self, "Error", "HufConverter.exe not found.")
            return

        op_map = {
            ("Translation File (.HUF)", "Excel Workbook (.XLSX)"): "huf2xls",
            ("Translation File (.HUF)", "Unicode Text (.TXT) (UTF-16 LE BOM)"): "huf2txt",
            ("Translation File (.HUF)", "Comma Separated (.CSV) (UTF-8 BOM)"): "huf2csv",
            ("Translation File (.HUF)", "Semicolon Separated (.CSV) (UTF-8 BOM)"): "huf2csv",
            ("Translation File (.HUF)", "Tab Separated (.TSV) (UTF-8 BOM)"): "huf2tsv",
            ("Translation File (.HUF)", "Custom Translation (.TR) (UTF-8 BOM)"): "huf2tr",
            ("Excel Workbook (.XLSX)", "Translation File (.HUF)" ): "xlsx2huf",
            ("Unicode Text (.TXT)", "Translation File (.HUF)" ): "txt2huf",
            ("Comma Separated (.CSV)", "Translation File (.HUF)" ): "csv2huf",
            ("Semicolon Separated (.CSV)", "Translation File (.HUF)" ): "csv2huf",
            ("Tab Separated (.TSV)", "Translation File (.HUF)" ): "tsv2huf",
            ("Custom Translation (.TR)", "Translation File (.HUF)" ): "tr2huf"
        }
        
        input_fmt = self.inputFormatCombo.currentText()
        output_fmt = self.outputFormatCombo.currentText()
        
        operation = op_map.get((input_fmt, output_fmt))
        if not operation:
            QMessageBox.critical(self, "Error", "Unsupported conversion type.")
            return
        
        args = [exe_path, operation, "-i", input_path]

        if output_path:
            args += ["-o", output_path]

        game = self.gameCombo.currentText()
        if game == "Total Club Manager 2005":
            args += ["-game", "tcm2005"]
        elif game == "FIFA Manager 06 - FIFA Manager 08":
            args += ["-game", "fm06"]

        if ";" in input_fmt or ";" in output_fmt:
            args += ["-separator", ";"]

        if not self.matchKeysCheck.isChecked():
            args += ["-keys", "none"]

        self.setDisabled(True)
        try:
            result = subprocess.run(args, check=False)
            if result.returncode == 0:
                QMessageBox.information(self, "Success", "Conversion completed successfully.")
            else:
                QMessageBox.critical(self, "Failure", "Conversion failed.")
        except Exception as e:
            QMessageBox.critical(self, "Error", str(e))
        finally:
            self.setDisabled(False)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    window = HufConverterGUI()
    window.show()
    sys.exit(app.exec_())
