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
        self.setWindowTitle("HufConverterGUI by Dmitri (1.03)")
        self.setFixedSize(600, 400)
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
        browseBtn.setFixedHeight(24)
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
        
        # Output Folder
        output_folder_label = QLabel("Output Dir (Optional):")
        output_folder_label.setFont(font)
        layout.addWidget(output_folder_label, 4, 0)
        self.outputFolderEdit = QLineEdit()
        self.outputFolderEdit.setFont(font)
        layout.addWidget(self.outputFolderEdit, 4, 1)
        browseOutputBtn = QPushButton("Browse")
        browseOutputBtn.setFont(font)
        browseOutputBtn.setFixedHeight(24)
        browseOutputBtn.clicked.connect(self.browse_output_folder)
        layout.addWidget(browseOutputBtn, 4, 2)

        # Output Filename (Optional)
        output_file_label = QLabel("Output Name (Optional):")
        output_file_label.setFont(font)
        layout.addWidget(output_file_label, 5, 0)
        self.outputFileEdit = QLineEdit()
        self.outputFileEdit.setFont(font)
        layout.addWidget(self.outputFileEdit, 5, 1, 1, 2)
        
        # Language combobox
        self.languageCombo = QComboBox()
        self.languageCombo.setFont(font)
        self.languageCombo.addItems(["English", "French", "German", "Italian", "Spanish", "Polish"])
        self.languageLabel = QLabel("Language:")
        self.languageLabel.setFont(font)
        self.languageLabel.setFixedWidth(80)
        layout.addWidget(self.languageLabel, 6, 0)
        layout.addWidget(self.languageCombo, 6, 1, 1, 2)
        
        # Match Keys
        self.matchKeysCheck = QCheckBox("Match Key Names")
        self.matchKeysCheck.setFont(font)
        self.matchKeysCheck.setChecked(True)
        self.matchKeysCheck.stateChanged.connect(self.match_keys_checked)
        layout.addWidget(self.matchKeysCheck, 7, 0)
        self.keysFileEdit = QLineEdit()
        self.keysFileEdit.setFont(font)
        self.keysFileEdit.setText("keys.txt")
        layout.addWidget(self.keysFileEdit, 7, 1)
        self.browseKeysBtn = QPushButton("Browse")
        self.browseKeysBtn.setFont(font)
        self.browseKeysBtn.setFixedHeight(24)
        self.browseKeysBtn.clicked.connect(self.browse_keys_file)
        layout.addWidget(self.browseKeysBtn, 7, 2)
        
        # Charmap File
        charmap_label = QLabel("Charmap File (Optional):")
        charmap_label.setFont(font)
        layout.addWidget(charmap_label, 8, 0)
        self.charmapFileEdit = QLineEdit()
        self.charmapFileEdit.setFont(font)
        layout.addWidget(self.charmapFileEdit, 8, 1)
        browseCharmapBtn = QPushButton("Browse")
        browseCharmapBtn.setFont(font)
        browseCharmapBtn.setFixedHeight(24)
        browseCharmapBtn.clicked.connect(self.browse_charmap_file)
        layout.addWidget(browseCharmapBtn, 8, 2)

        # Row layout for checkboxes
        rowLayout = QHBoxLayout()

        # Write Hashes
        self.hashesCheck = QCheckBox("Write Hashes")
        self.hashesCheck.setFont(font)
        self.hashesCheck.setChecked(False)
        rowLayout.addWidget(self.hashesCheck)
        
        # Windows-1251
        self.windows1251check = QCheckBox("Windows-1251")
        self.windows1251check.setFont(font)
        self.windows1251check.setChecked(False)
        rowLayout.addWidget(self.windows1251check)
        
        # Show Stats
        self.statsCheck = QCheckBox("Show Stats")
        self.statsCheck.setFont(font)
        self.statsCheck.setChecked(False)
        rowLayout.addWidget(self.statsCheck)
        
        layout.addLayout(rowLayout, 9, 0, 1, 3)
        
        # Convert Button
        self.convertBtn = QPushButton("Convert")
        self.convertBtn.setFont(largeFont)
        self.convertBtn.setFixedHeight(40)
        self.convertBtn.clicked.connect(self.convert_file)
        layout.addWidget(self.convertBtn, 10, 0, 1, 3)

        self.setLayout(layout)
        self.inputFormatCombo.setCurrentIndex(0)
        self.update_output_format()

    def browse_input_file(self):
        fileName, _ = QFileDialog.getOpenFileName(self, "Select Input File")
        if fileName:
            self.inputFileEdit.setText(fileName)

    def browse_output_folder(self):
        folder = QFileDialog.getExistingDirectory(self, "Select Output Folder") 
        if folder:
            self.outputFolderEdit.setText(folder)

    def browse_keys_file(self):
        fileName, _ = QFileDialog.getOpenFileName(self, "Select Input File")
        if fileName:
            self.keysFileEdit.setText(fileName)

    def match_keys_checked(self, state):
        self.keysFileEdit.setEnabled(state != Qt.Unchecked)
        self.browseKeysBtn.setEnabled(state != Qt.Unchecked)

    def browse_charmap_file(self):
        fileName, _ = QFileDialog.getOpenFileName(self, "Select Character Mapping File")
        if fileName:
            self.charmapFileEdit.setText(fileName)

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
        output_filename = self.outputFileEdit.text().strip()
        output_folder = self.outputFolderEdit.text().strip()

        if not os.path.isfile(input_path):
            QMessageBox.critical(self, "Error", "Invalid input file.")
            return

        if output_folder and not os.path.isdir(output_folder):
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
            ("Excel Workbook (.XLSX)", "Translation File (.HUF)"): "xlsx2huf",
            ("Unicode Text (.TXT)", "Translation File (.HUF)"): "txt2huf",
            ("Comma Separated (.CSV)", "Translation File (.HUF)"): "csv2huf",
            ("Semicolon Separated (.CSV)", "Translation File (.HUF)"): "csv2huf",
            ("Tab Separated (.TSV)", "Translation File (.HUF)"): "tsv2huf",
            ("Custom Translation (.TR)", "Translation File (.HUF)"): "tr2huf"
        }

        input_fmt = self.inputFormatCombo.currentText()
        output_fmt = self.outputFormatCombo.currentText()

        operation = op_map.get((input_fmt, output_fmt))
        if not operation:
            QMessageBox.critical(self, "Error", "Unsupported conversion type.")
            return

        ext_map = {
            "Excel Workbook (.XLSX)": ".xlsx",
            "Unicode Text (.TXT) (UTF-16 LE BOM)": ".txt",
            "Comma Separated (.CSV) (UTF-8 BOM)": ".csv",
            "Semicolon Separated (.CSV) (UTF-8 BOM)": ".csv",
            "Tab Separated (.TSV) (UTF-8 BOM)": ".tsv",
            "Custom Translation (.TR) (UTF-8 BOM)": ".tr",
            "Translation File (.HUF)": ".huf",
        }
        out_ext = ext_map.get(output_fmt, "")

        output_path = ""
        if output_folder and output_filename:  
            name, ext = os.path.splitext(output_filename)
            if not ext:
                output_filename += out_ext
            output_path = os.path.join(output_folder, output_filename)
        elif output_folder and not output_filename:  
            input_name = os.path.splitext(os.path.basename(input_path))[0]
            output_path = os.path.join(output_folder, input_name + out_ext)
        elif output_filename and not output_folder:  
            name, ext = os.path.splitext(output_filename)
            if not ext:
                output_filename += out_ext
            output_path = output_filename

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

        if self.matchKeysCheck.isChecked() and self.keysFileEdit.text().strip():
            args += ["-keys", self.keysFileEdit.text().strip()]
        else:
            args += ["-keys", "none"]

        if self.charmapFileEdit.text().strip():
            args += ["-charmap", self.charmapFileEdit.text().strip()]

        if self.hashesCheck.isChecked():
            args += ["-hashes"]

        if self.windows1251check.isChecked():
            args += ["-windows1251"]

        if self.statsCheck.isChecked():
            args += ["-stats"]

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
            #import shlex
            #print(" ".join(shlex.quote(a) for a in args))
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
