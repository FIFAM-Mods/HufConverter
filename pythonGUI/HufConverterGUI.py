import sys
import os
import subprocess
import platform
import struct
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
        self.setWindowTitle("HufConverterGUI by Dmitri (1.00)")
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

        # Row layout for checkboxes + language
        rowLayout = QHBoxLayout()

        # Match Keys
        self.matchKeysCheck = QCheckBox("Match Key Names")
        self.matchKeysCheck.setFont(font)
        self.matchKeysCheck.setChecked(True)
        rowLayout.addWidget(self.matchKeysCheck)

        # Write Hashes
        self.hashesCheck = QCheckBox("Write Hashes")
        self.hashesCheck.setFont(font)
        self.hashesCheck.setChecked(False)
        rowLayout.addWidget(self.hashesCheck)

        # Language combobox
        self.languageCombo = QComboBox()
        self.languageCombo.setFont(font)
        self.languageCombo.addItems(["English", "French", "German", "Italian", "Spanish", "Polish"])
        self.languageLabel = QLabel("Language:")
        self.languageLabel.setFont(font)
        self.languageLabel.setFixedWidth(80)
        rowLayout.addWidget(self.languageLabel)
        rowLayout.addWidget(self.languageCombo)

        layout.addLayout(rowLayout, 5, 0, 1, 3)
        
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
        path = self.inputFileEdit.text().strip().lower()
        ext_map = {
            ".huf": "Translation File (.HUF)",
            ".xlsx": "Excel Workbook (.XLSX)",
            ".txt": "Unicode Text (.TXT)",
            ".csv": None,
            ".tsv": "Tab Separated (.TSV)",
            ".tr": "Custom Translation (.TR)"
        }

        for ext, name in ext_map.items():
            if path.endswith(ext):
                if ext == ".csv":
                    sep = self._detect_csv_separator(self.inputFileEdit.text())
                    if sep == ";":
                        self.inputFormatCombo.setCurrentText("Semicolon Separated (.CSV)")
                    else:
                        self.inputFormatCombo.setCurrentText("Comma Separated (.CSV)")
                else:
                    self.inputFormatCombo.setCurrentText(name)

                full_path = self.inputFileEdit.text()
                if ext == ".huf" and os.path.isfile(full_path):
                    try:
                        if os.path.getsize(full_path) >= 12:
                            with open(full_path, "rb") as f:
                                header = f.read(12)
                                if header[:4] == b"CLFB":
                                    self.gameCombo.setCurrentText("FIFA Manager 09 - FIFA Manager 14")
                                elif self.gameCombo.currentText() == "FIFA Manager 09 - FIFA Manager 14":
                                    self.gameCombo.setCurrentText("FIFA Manager 06 - FIFA Manager 08")

                                import struct
                                lang_id = struct.unpack("<I", header[8:12])[0]
                                if 1 <= lang_id <= 6:
                                    self.languageCombo.setCurrentIndex(lang_id - 1)
                    except Exception:
                        pass
                break

    def _detect_csv_separator(self, filepath):
        try:
            with open(filepath, "r", encoding="utf-8-sig") as f:
                for line in f:
                    line = line.strip()
                    if not line:
                        continue
                    comma_count = 0
                    semicolon_count = 0
                    in_quotes = False
                    for ch in line:
                        if ch == '"':
                            in_quotes = not in_quotes
                        elif not in_quotes:
                            if ch == ',':
                                comma_count += 1
                            elif ch == ';':
                                semicolon_count += 1
                    if semicolon_count > comma_count:
                        return ";"
                    else:
                        return ","
        except Exception:
            return ","


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

        base_dir = os.path.dirname(sys.executable if getattr(sys, 'frozen', False) else __file__)
        exe_path = os.path.join(base_dir, "HufConverter.exe")
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

        if "Semicolon" in input_fmt or "Semicolon" in output_fmt:
            args += ["-separator", ";"]

        if not self.matchKeysCheck.isChecked():
            args += ["-keys", "none"]

        if self.hashesCheck.isChecked():
            args += ["-hashes"]
            
        language_id = self.languageCombo.currentIndex() + 1
        args += ["-language", str(language_id)]

        args_file = os.path.join(base_dir, "arguments.txt")
        if os.path.isfile(args_file):
            try:
                with open(args_file, "r", encoding="utf-8") as f:
                    first_line = f.readline().strip()
                    if first_line:
                        args += first_line.split()
            except Exception:
                pass

        self.setDisabled(True)
        QApplication.processEvents()
        try:
            if platform.system() == "Windows":
                si = subprocess.STARTUPINFO()
                si.dwFlags |= subprocess.STARTF_USESHOWWINDOW
                result = subprocess.run(
                    args,
                    check=False,
                    startupinfo=si,
                    creationflags=subprocess.CREATE_NO_WINDOW
                )
            else:
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
