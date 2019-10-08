# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)

import unittest
import numpy as np

from MsdTestHelper import (is_registered, check_output, do_a_fit)


class SCgapSwaveTest(unittest.TestCase):

	def test_function_has_been_registered(self):
		status, msg = is_registered("SCgapSwave")
		if not status:
			self.fail(msg)

	def test_function_output(self):
		input = [0.1, 3.1, 6.1, 9.1]
		expected = [1.0, 0.9514971, 0.53404999, 0.0]
		tolerance = 1.0e-05
		status, output = check_output("SCgapSwave", input, expected, tolerance, Delta = 1.2, Tcritical = 9.0)
		if not status:
			msg = 'Computed output {} from input {} unequal to expected: {}'
			self.fail(msg.format(*[str(i) for i in (output, input, expected)]))

	def test_do_fit(self):
		do_a_fit(np.arange(0.1, 16, 0.2), 'SCgapSwave', guess = dict( = 1.25, Tcritical = 9.1), target = dict( = 1.2, Tcritical = 9.0), atol = 0.01)

if __name__ == '__main__':
	unittest.main()
