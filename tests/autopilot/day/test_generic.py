from autopilot.testcase import AutopilotTestCase
import subprocess
import os
import locale
import configparser
import json

dir_base = os.path.expanduser("~/.local/share/unity-scopes")
scope_id = "com.canonical.scopes.day_day"
click_id = "com.canonical.scopes.day"
scope_path = os.path.join(dir_base, scope_id + "_1.5.2")
cache_path = os.path.join(dir_base, 'leaf-net/' + click_id)
ini_path = os.path.join(scope_path, scope_id + ".ini")
new_mo_path = os.path.join("/opt/click.ubuntu.com", click_id, "1.5.2", "share")

#POFILES arrives from cmake as a semicolon delimited string of paths to all po files
langs = 'po/en_GB.po;po/es.po;po/sr.po;po/zh_TW.po;po/fi.po;po/bg.po;po/it.po;po/ca.po;po/sv.po;po/uk.po;po/sl.po;po/pt_BR.po;po/sk.po;po/hu.po;po/zh_CN.po;po/el.po;po/gl.po;po/fr.po;po/cs.po;po/gd.po;po/de.po;po/da.po;po/pl.po;po/eu.po;po/nb.po;po/nl.po;po/pt.po;po/ru.po;po/is.po'.replace('.po', '')
langs = langs.replace('po/', '')
langs = langs.split(';')

SCOPEDATA = '/home/phablet/autopilot/scope-data'
SCOPEDATA_ARGS =  ["--lat", "51.506", "--long", "-0.099", "--country", "es"]
if ():
    SCOPEDATA_ARGS.append('--set-connected');
encoding = locale.getdefaultlocale()[1]

scope_list_b = subprocess.check_output([SCOPEDATA, "--list"])
scope_list = list(scope_list_b.decode(encoding).split('\n'))

class GenericScopeTests(AutopilotTestCase):
    """
    Test scopes stuff
    """

    def has_ini_keyvalue(self, section, key, isFile=False):
        """
        Not a test, a helper method.

        Verifies the scope ini card has a key in the named section, that the key has a value, and if
        the value points to a file relative to the scope dir, that the file exists.
        """
        parser = configparser.ConfigParser()
        try:
            with open(ini_path, 'r') as ini:
                parser.read_file(ini)
        except:
            self.fail(ini_path + 'not openable or corrupted')

        try:
            val = parser.get(section, key)
        except configparser.NoOptionError:
            self.fail('Scope ini lacks %s key' % key)
        self.assertNotEqual(len(val), 0, 'Scope ini file %s key has no value' % key)

        if isFile:
            val = val.lstrip("./")
            val_path = os.path.join(scope_path, val)
            self.assertTrue(os.path.isfile(val_path), val_path + ' does not exist')

    def test_clickapp_installed(self):
        """Verify the click app is actually installed."""

        installed_apps = subprocess.check_output(['click', 'list']).decode()
        self.assertTrue(click_id in installed_apps, click_id + ' is not reported as installed by click')

    def test_in_scope_list(self):
        """Test that the scope shows up in the scope registry list"""

        self.assertTrue('com.canonical.scopes.day_day' in scope_list, 'com.canonical.scopes.day_day not returned by %s --list' % SCOPEDATA )

    def test_scope_dir_exists(self):
        """Verify the scope's directory exists here: /home/phablet/.local/share/unity-scopes/SCOPE"""

        self.assertTrue(os.path.isdir(scope_path), scope_path + ' does not exist')

    def test_scope_cache_dir_exists(self):
        """Verify the scope's cache dir exists"""

        #run the scope once to ensure the cache dir will be created
        cmd = [SCOPEDATA, scope_id, "--results"]
        cmd += SCOPEDATA_ARGS
        subprocess.check_output(cmd)
        cache_base = os.path.expanduser("~/.local/share/unity-scopes/leaf-net")
        cache_base_unconfined = os.path.expanduser("~/.local/share/unity-scopes/unconfined")
        cache_path = os.path.join(cache_base, click_id)
        cache_path_unconfined = os.path.join(cache_base_unconfined, scope_id)
        cached = os.path.isdir(cache_path) or os.path.isdir(cache_path_unconfined)
        self.assertTrue(cached, cache_path + ' or ' + cache_path_unconfined + ' does not exist')

    def test_ini_file_exists(self):
        """Verify the ini file exists in the expected place"""

        self.assertTrue(os.path.isfile(ini_path), ini_path + ' does not exist')

    def test_executable_exists(self):
        """Verify the go executable or .so exists"""

        if False:
            exec_path = os.path.join(scope_path, scope_id)
            self.assertTrue(os.path.isfile(exec_path), exec_path + ' does not exist')
            self.assertTrue(os.access(exec_path, os.X_OK), 'access error on ' + exec_path)
        else:
            lib = "lib" + scope_id + ".so"
            so_path = os.path.join(scope_path, lib)
            self.assertTrue(os.path.isfile(so_path), so_path + ' does not exist')

    def test_ini_scopeconfig_fields_untranslated(self):
        """Verify the ini file has some REQUIRED key/value pairs"""

        self.has_ini_keyvalue('ScopeConfig', 'DisplayName')
        self.has_ini_keyvalue('ScopeConfig', 'Description')
        self.has_ini_keyvalue('ScopeConfig', 'Icon', True)
        if False:
            self.has_ini_keyvalue('ScopeConfig', 'SearchHint')
        if True:
            self.has_ini_keyvalue('ScopeConfig', 'LocationDataNeeded')

    def test_ini_scopeconfig_fields_translated(self):
        """
        TODO: update this. Verify scope's REQUIRED metadata is localized to target languages: DisplayName,
        Description and SearchHint (if used as TEST_SEARCH_HINT is True through cmake)
        """
        if True:
            for lang in langs:
                if True:
                    self.has_ini_keyvalue('ScopeConfig', 'DisplayName[' + lang + ']')
                if False:
                    self.has_ini_keyvalue('ScopeConfig', 'SearchHint[' + lang + ']')

    def test_ini_appearance_fields(self):
        """Verify the ini file has required Appearance key/value pairs"""

        if False:
            self.has_ini_keyvalue('Appearance', 'PageHeader.Logo', True)
        if False:
            self.has_ini_keyvalue('Appearance', 'PageHeader.Background')
        if False:
            self.has_ini_keyvalue('Appearance', 'PageHeader.ForegroundColor')
        if False:
            self.has_ini_keyvalue('Appearance', 'PageHeader.DividerColor')
        if False:
            self.has_ini_keyvalue('Appearance', 'PreviewButtonColor')

    def test_mos_exist(self):
        """Verify mo files exist for all target langs"""
        if True:
            for lang in langs:
                dir_path = os.path.join(scope_path, 'locale', lang, 'LC_MESSAGES')
                new_dir_path = os.path.join(new_mo_path, 'locale', lang, 'LC_MESSAGES')
                mopath = os.path.join(dir_path, 'day.mo')
                new_mopath = os.path.join(new_dir_path, 'day.mo')
                mofolderexist = os.path.isdir(dir_path) or os.path.isdir(new_dir_path)
                self.assertTrue(mofolderexist, 'Directory for mo file does not exist: neither %s nor %s' % (str(dir_path), str(new_dir_path)))
                moexist = os.path.isfile(mopath) or os.path.isfile(new_mopath)
                self.assertTrue(moexist, 'mo file does not exist: neither %s nor %s' % (str(mopath), str(new_mopath)))

    def test_results_surfaced(self):
        """
        Verify results are surfaced (when no query string is set). Makes
        sure that the number of results >= 1
        """
        if True:
            cmd = [SCOPEDATA, scope_id, "--results"]
            cmd += SCOPEDATA_ARGS
            outp = subprocess.check_output(cmd)
            outstring = outp.decode(encoding)
            try:
                a = json.loads(outstring)
            except ValueError:
                self.fail('json format error within %s' % outstring)
            self.assertTrue(len(a) > 0, 'No results returned by: %s' % ' '.join(cmd))

    def test_search(self):
        """Verifies a query returns a result with a specified search query"""
        if False:
            cmd = [SCOPEDATA, scope_id, "--search", '',"--results"]
            cmd += SCOPEDATA_ARGS
            outp = subprocess.check_output(cmd)
            outstring = outp.decode(encoding)
            try:
                a = json.loads(outstring)
            except ValueError:
                self.fail('json format error within %s' % outstring)
            self.assertTrue(len(a) > 0, 'was expecting a result but got none with search string %s' % '')

    def test_departments(self):
        """Verifies thare are more than zero departments"""
        if False:
            cmd = [SCOPEDATA, scope_id, "--departments"]
            cmd += SCOPEDATA_ARGS
            outp = subprocess.check_output(cmd)
            outstring = outp.decode(encoding)
            try:
                a = json.loads(outstring)
            except ValueError:
                self.fail('No Departments found')
            self.assertTrue(len(a["subdepartments"]) > 0, 'was expecting Departments but got none')

    def test_departments_surface(self):
        """Verifies thare are more than zero departments"""
        if False:
            cmd = [SCOPEDATA, scope_id, "--departments"]
            cmd += SCOPEDATA_ARGS
            outp = subprocess.check_output(cmd)
            outstring = outp.decode(encoding)
            try:
                a = json.loads(outstring)
            except ValueError:
                self.fail('No Departments found')
            faileddep=[]
            for subdep in a["subdepartments"]:
                cmd = [SCOPEDATA, "scope://{0}?dep={1}".format(scope_id, subdep["id"]), "--results"]
                cmd += SCOPEDATA_ARGS
                outp = subprocess.check_output(cmd)
                outstring = outp.decode(encoding)
                if len(outstring) > 1 and outstring[0] != '[' and outstring.find('[') != -1:
                    outstring = outstring[outstring.index('['):]
                try:
                    a = json.loads(outstring)
                except ValueError:
                    self.fail('No results returned by: %s' % outstring)
                if len(a) <= 0:
                    faileddep.append(subdep["id"])
            self.assertTrue(len(faileddep) == 0, 'No result returned for departments: %s' % ' '.join(faileddep))
