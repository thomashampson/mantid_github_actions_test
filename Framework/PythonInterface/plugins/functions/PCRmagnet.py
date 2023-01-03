# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
# pylint: disable=invalid-name, anomalous-backslash-in-string, attribute-defined-outside-init

from mantid.api import IFunction1D, FunctionFactory
import numpy as np


class PCRmagnet(IFunction1D):
    def category(self):
        return "Muon\\MuonSpecific"

    def init(self):
        self.declareParameter("A0", 0.2, "Amplitude")
        self.declareParameter("Sigma", 0.1, "Relaxation rate")
        self.declareParameter("H0", 3.0, "Local field")
        self.declareParameter("Toff", 0.1, "Time offset")

    def function1D(self, x):
        A0 = self.getParameterValue("A0")
        sigma = self.getParameterValue("Sigma")
        H0 = self.getParameterValue("H0")
        Toff = self.getParameterValue("Toff")
        gmu = 2 * np.pi * 0.01355342
        w = H0 * gmu
        x = x - Toff
        return A0 * (1.0 / 3.0 + 2.0 / 3.0 * np.exp(-((sigma * x) ** 2) / 2) * np.cos(w * x))


FunctionFactory.subscribe(PCRmagnet)
