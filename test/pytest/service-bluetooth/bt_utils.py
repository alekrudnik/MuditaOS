import pytest
from harness import log
from harness.interface.defs import key_codes
from harness.dom_parser_utils import *
import time

@pytest.fixture(scope='function')
def bt_main_window(harness):
    current_window_content = get_window_content(harness, 1)
    assert item_contains_recursively(current_window_content, 'WindowName', 'MainWindow' )
    harness.connection.send_key_code(key_codes["enter"])

    log.info("Navigating to ApplicationSettings")
    harness.open_application("settings")
    if harness.connection.get_application_name() != "ApplicationSettingsNew":
        time.sleep(5)
        assert harness.connection.get_application_name() == "ApplicationSettingsNew"

    log.info("Opening Bluetooth")
    harness.connection.send_key_code(key_codes["down"])
    harness.connection.send_key_code(key_codes["enter"])

@pytest.fixture(scope='function')
def bt_reset(harness):
    current_window_content = get_window_content(harness, 1)
    assert item_contains_recursively(current_window_content, 'WindowName', 'Bluetooth' )

    parent_of_list_items = find_parent(current_window_content, 'ListItem')
    if item_has_child_that_contains_recursively( parent_of_list_items, [('TextValue','Bluetooth'), ('TextValue', 'ON')] ) :
        log.info("Bluetooth is ON, turing OFF...")
        harness.connection.send_key_code(key_codes["enter"])

    current_window_content = get_window_content(harness, 1)
    parent_of_list_items = find_parent(current_window_content, 'ListItem')
    assert item_has_child_that_contains_recursively( parent_of_list_items, [('TextValue','Bluetooth'), ('TextValue', 'OFF')] )

    log.info("Turing Bluetooth ON...")
    harness.connection.send_key_code(key_codes["enter"])

@pytest.fixture(scope='function')
def bt_all_devices(harness):
    log.info("Navigating to AllDevices window...")
    time.sleep(1)
    harness.connection.send_key_code(key_codes["down"])
    harness.connection.send_key_code(key_codes["enter"])

    current_window_content = get_window_content(harness, 1)
    assert item_contains_recursively(current_window_content, 'WindowName', 'AllDevices')
