import unittest
import os

from mantid import ConfigService, config, std_vector_str

class ConfigServiceTest(unittest.TestCase):

    __dirs_to_rm = []
    __init_dir_list = ''
    
    def test_singleton_returns_instance_of_ConfigService(self):
        self.assertTrue(isinstance(config, ConfigService))
        
    def test_service_acts_like_dictionary(self):
        self.assertTrue('plugins.directory' in config)
        self.assertNotEqual(config['plugins.directory'], "")

    def test_getting_search_paths(self):
        """Retrieve the search paths
        """
        paths = config.get_data_dirs()
        self.assertEquals(type(paths), std_vector_str)
        self.assert_(len(paths) > 0)

#    def test_setting_data_search_paths_via_string(self):
#        """Set data search paths via a string
#        """
#        updated = self._setup_test_areas()
#        # Set by a string
#        config.set_data_dirs(updated)
#        # The test
#        self._do_path_test()
#        self._clean_up_test_areas()
#
#    def test_setting_data_search_paths_via_list(self):
#        """Set data search paths via a list
#        """
#        updated = self._setup_test_areas()
#        updated_list = updated.split(';')
#        # Set by a list
#        mtd.settings.setDataSearchDirs(updated_list)
#        # The test
#        self._do_path_test()
#        self._clean_up_test_areas()
#        
    def _do_path_test(self):
        """Perform the path test
        """
        # Have they been updated - The stored values come back with trailing slashes
        new_value = mtd.settings.get_data_dirs()
        self.assertEquals(len(new_value), 2)
        self.assert_('tmp' in new_value[0])
        self.assert_('tmp_2' in new_value[1])

    def _setup_test_areas(self):
        """Set data search paths via a list
        """
        self.__init_dir_list = config['datasearch.directories']
        # Set new paths - Make a temporary directory so that I know where it is
        test_path = os.path.join(os.getcwd(), "tmp")
        try:
            os.mkdir(test_path)
            self.__dirs_to_rm.append(test_path)
        except OSError:
            pass

        test_path_two = os.path.join(os.getcwd(), "tmp_2")
        try:
            os.mkdir(test_path_two)
            self.__dirs_to_rm.append(test_path_two)
        except OSError:
            pass
        
        updated = test_path + '/;' + test_path_two + '/'
        return updated

    def _clean_up_test_areas(self):
        config['datasearch.directories'] = self.__init_dir_list
        
        # Remove temp directories
        for p in self.__dirs_to_rm:
            try:
                os.rmdir(p)
            except OSError:
                pass

