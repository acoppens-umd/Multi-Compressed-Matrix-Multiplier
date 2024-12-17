from io import StringIO
from zipfile import sizeEndCentDir

from scipy.io import mmread
from scipy.sparse import csr_matrix, csc_matrix
import numpy as np
import sys
import csv

import matplotlib.pyplot as plt

class DenseDiag:
    def __init__(self, arr, i, j):
        self.arr = arr
        self.i = i
        self.j = j

class DenseBlock:
    def __init__(self, arr, i1, i2, j1, j2):
        self.arr = arr
        self.i1 = i1
        self.i2 = i2
        self.j1 = j1
        self.j2 = j2


def density(arr):
    return np.count_nonzero(arr) / arr.size

def __main__():
    diffs = []

    for arg in sys.argv[1:]:
        print("For Matrix " + arg)

        with open(arg) as f:
            m=mmread(f)

            m_arr = m.toarray()

            m_rows = m_arr.shape[0]
            m_cols = m_arr.shape[1]

            dense_size = m_rows * m_cols

            print("Rows: " + str(m_rows) + " Cols: " + str(m_cols))

            print("Dense size: " + str(dense_size))

            diagonal_threshold = .5
            block_threshold = .5

            available_diags = list(range(-m_rows + 1, m_cols))
            next_available_diags = available_diags.copy()

            available_horiz_blocks = list(range(m_rows))
            next_available_horiz_blocks = available_horiz_blocks.copy()

            available_vert_blocks = list(range(m_cols))
            next_available_vert_blocks = available_vert_blocks.copy()

            dense_elements = []

            while np.any(m_arr):
                for diag_offset in available_diags:
                    diagonal = np.diagonal(m_arr, offset=diag_offset)
                    if density(diagonal) > diagonal_threshold:
                        if diag_offset > 0:
                            dense_elements.append(DenseDiag(np.copy(diagonal), 0, diag_offset))
                            np.fill_diagonal(m_arr[:, diag_offset:], 0)
                        else:
                            dense_elements.append(DenseDiag(np.copy(diagonal), -diag_offset, 0))
                            np.fill_diagonal(m_arr[-diag_offset:, :], 0)

                    if not np.any(np.diagonal(m_arr, offset=diag_offset)):
                        next_available_diags.remove(diag_offset)

                diagonal_threshold = diagonal_threshold / 2
                available_diags = next_available_diags.copy()

                for row_offset in available_horiz_blocks:
                    row = m_arr[row_offset]

                    if density(row) > block_threshold:
                        dense_elements.append(DenseBlock(np.copy(row), row_offset, row_offset + 1, 0, m_cols))
                        m_arr[row_offset] = 0

                    if not np.any(m_arr[row_offset]):
                        next_available_horiz_blocks.remove(row_offset)

                available_horiz_blocks = next_available_horiz_blocks.copy()

                for col_offset in available_vert_blocks:
                    col = m_arr[:,col_offset]

                    if density(col) > block_threshold:
                        dense_elements.append(DenseBlock(np.copy(col), col_offset, col_offset + 1, 0, m_rows))
                        m_arr[:,col_offset] = 0

                    if not np.any(m_arr[:,col_offset]):
                        next_available_vert_blocks.remove(col_offset)

                available_vert_blocks = next_available_vert_blocks.copy()

                block_threshold = block_threshold / 2

                #plt.imshow(m_arr)
                #plt.show()

            multi_compressed_size = 0
            for element in dense_elements:
                if isinstance(element, DenseDiag):
                    multi_compressed_size += len(element.arr) + 2

                if isinstance(element, DenseBlock):
                    multi_compressed_size += len(element.arr) + 4

            print("Multi Compressed size: " + str(multi_compressed_size) + " CR: "
                  + str(multi_compressed_size / dense_size))

            m_csr = csr_matrix(m)

            csr_size = len(m_csr.data) + len(m_csr.indptr) + len(m_csr.indices)

            print("CSR size: " + str(csr_size) + " CR: " + str(csr_size / dense_size))

            with open(arg + ".csv", 'w', newline='') as csv_file:
                writer = csv.writer(csv_file)
                writer.writerows([[m_rows, m_cols], m_csr.indptr, m_csr.data, m_csr.indices])

            diff = multi_compressed_size / csr_size
            print("Diff: " + str(diff))

            diffs.append(diff)

    geomean_diff = np.prod(diffs) ** (1/len(diffs))

    print("Geomean diff: " + str(geomean_diff))


__main__()