from openpyxl import Workbook
from openpyxl import load_workbook
from openpyxl.utils import get_column_letter


dest_filename = 'analysis.xlsx'
wb = load_workbook(filename = 'analysis.xlsx')

sheet_ranges = wb['Messung']
# ws = wb.create_sheet(title="Messung")
sheet_ranges['A1'] = 123456

print(sheet_ranges['A1'].value)

wb.save(filename = dest_filename)