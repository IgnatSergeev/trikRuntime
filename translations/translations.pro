# Copyright 2015 CybrTech Labs Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

TEMPLATE = subdirs

include(../global.pri)

win32 {
	DESTDIR ~= s,/,\,g
	system(cmd /C "xcopy *.qm $$DESTDIR\\translations\\ /s /e /y")
	system(cmd /C "xcopy *.ini $$DESTDIR\\translations\\ /s /e /y")
}
else {
	system(mkdir -p $$DESTDIR/translations/; find ./ -name *.qm -exec cp --parents {} $$DESTDIR/translations \\;)
	system(mkdir -p $$DESTDIR/translations/; find ./ -name *.ini -exec cp --parents {} $$DESTDIR/translations \\;)
}

OTHER_FILES += \
	$$PWD/ru/locale.ini \
	$$PWD/fr/locale.ini \

OTHER_FILES += \
	$$PWD/ru/*.ts \
	$$PWD/fr/*.ts \

installAdditionalSharedFiles($$DESTDIR/translations)
