/* Copyright 2014 - 2015 CyberTech Labs Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include <QsLog.h>
#include <QTimer>
#include "trikScriptRunner.h"
#include "trikJavaScriptRunner.h"
#include "trikPythonRunner.h"

#include <trikKernel/timeVal.h>
#include "threading.h"
#include <QMetaMethod>

using namespace trikControl;
using namespace trikScriptRunner;
using namespace trikNetwork;
using namespace trikKernel;

Q_DECLARE_METATYPE(QVector<uint8_t>)
Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QTimer*)

/// Constructor.
/// @param brick - reference to trikControl::Brick instance.
/// @param mailbox - mailbox object used to communicate with other robots.
TrikScriptRunner::TrikScriptRunner(trikControl::BrickInterface &brick
								   , trikNetwork::MailboxInterface * const mailbox
								   )
	: brick(brick), mailbox(mailbox), mLastRunner(ScriptType::JAVASCRIPT)
{
		REGISTER_DEVICES_WITH_TEMPLATE(REGISTER_METATYPE)
}

TrikScriptRunner::~TrikScriptRunner()
{
	abortAll();
}

void TrikScriptRunner::setDefaultRunner(ScriptType t)
{
	mLastRunner = t;
}

void TrikScriptRunner::registerUserFunction(const QString &name, QScriptEngine::FunctionSignature function)
{
	fetchRunner(mLastRunner)->registerUserFunction(name, function);
}

void TrikScriptRunner::addCustomEngineInitStep(const std::function<void (QScriptEngine *)> &step)
{
	fetchRunner(mLastRunner)->addCustomEngineInitStep(step);
}

bool TrikScriptRunner::wasError()
{
	return fetchRunner(mLastRunner)->wasError();
}

QStringList TrikScriptRunner::knownMethodNames() const
{
	return mScriptRunnerArray[to_underlying(mLastRunner)]->knownMethodNames();
}

QStringList TrikScriptRunner::knownMethodNamesFor(ScriptType r)
{
	return fetchRunner(r)->knownMethodNames();
}

void TrikScriptRunner::run(const QString &script, const QString &fileName)
{
	if (fileName.endsWith(".py")) {
		run(script, ScriptType::PYTHON, fileName);
	} else { // default JS
		run(script, ScriptType::JAVASCRIPT, fileName);
	}
}

TrikScriptRunnerInterface * TrikScriptRunner::fetchRunner(ScriptType stype)
{
	auto & cell = mScriptRunnerArray[to_underlying(stype)];
	if (cell ==  nullptr) { // lazy creation
		switch (stype) {
			case ScriptType::JAVASCRIPT:
				QScopedPointer<TrikScriptRunnerInterface>(new TrikJavaScriptRunner(brick, mailbox)).swap(cell);
				break;
			case ScriptType::PYTHON:
				QScopedPointer<TrikScriptRunnerInterface>(new TrikPythonRunner(brick, mailbox)).swap(cell);
				break;
			default:
				QLOG_ERROR() << "Can't handle script with unrecognized type: " << to_underlying(stype);
				return nullptr;
		}
		// subscribe on wrapped objects signals
		connect(&*cell, &TrikScriptRunnerInterface::completed, this, &TrikScriptRunnerInterface::completed);
		connect(&*cell, &TrikScriptRunnerInterface::startedScript, this, &TrikScriptRunnerInterface::startedScript);
		connect(&*cell, &TrikScriptRunnerInterface::startedDirectScript
				, this, &TrikScriptRunnerInterface::startedDirectScript);
		connect(&*cell, &TrikScriptRunnerInterface::sendMessage, this, &TrikScriptRunnerInterface::sendMessage);
	}

	setDefaultRunner(stype);

	return cell.data();
}

void TrikScriptRunner::run(const QString &script, ScriptType stype, const QString &fileName)
{
	abortAll(); // FIXME: or fetchRunner(stype)->abort()? or abort(/*last*/)?

	fetchRunner(stype)->run(script, fileName);
}

void TrikScriptRunner::runDirectCommand(const QString &command)
{
	fetchRunner(mLastRunner)->runDirectCommand(command);
}

void TrikScriptRunner::abort()
{
	fetchRunner(mLastRunner)->abort();
}

void TrikScriptRunner::abortAll()
{
	for (auto && r: mScriptRunnerArray) {
		if (r != nullptr) {
			r->abort();
		}
	}
}

void TrikScriptRunner::brickBeep()
{
	fetchRunner(mLastRunner)->brickBeep();
}
