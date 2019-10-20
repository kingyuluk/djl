/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may not use this file except in compliance
 * with the License. A copy of the License is located at
 *
 * http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
 * and limitations under the License.
 */
package ai.djl.examples.training.util;

import ai.djl.Device;
import ai.djl.engine.Engine;
import ai.djl.examples.util.MemoryUtils;
import ai.djl.examples.util.ProgressBar;
import ai.djl.metric.Metric;
import ai.djl.metric.Metrics;
import ai.djl.training.TrainingListener;
import ai.djl.zoo.ModelNotFoundException;
import java.io.IOException;
import java.util.List;
import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.HelpFormatter;
import org.apache.commons.cli.Options;
import org.apache.commons.cli.ParseException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public abstract class AbstractTraining implements TrainingListener {

    private static final Logger logger = LoggerFactory.getLogger(AbstractTraining.class);

    private float trainingAccuracy;
    private float trainingLoss;

    protected int trainDataSize;
    protected int validateDataSize;
    protected int trainingProgress;
    protected int validateProgress;

    private long epochTime;

    private ProgressBar trainingProgressBar;
    private ProgressBar validateProgressBar;

    protected Metrics metrics;

    public AbstractTraining() {
        metrics = new Metrics();
    }

    public boolean runExample(String[] args) {
        Options options = Arguments.getOptions();
        try {
            DefaultParser parser = new DefaultParser();
            CommandLine cmd = parser.parse(options, args, null, false);
            Arguments arguments = new Arguments(cmd);

            logger.info(
                    "Running {} on: {}, epoch: {}.",
                    getClass().getSimpleName(),
                    Device.defaultDevice(),
                    arguments.getEpoch());

            long init = System.nanoTime();
            String version = Engine.getInstance().getVersion();
            long loaded = System.nanoTime();
            logger.info(
                    String.format(
                            "Load library %s in %.3f ms.", version, (loaded - init) / 1_000_000f));

            train(arguments);

            float p50 = metrics.percentile("train", 50).getValue().longValue() / 1_000_000f;
            float p90 = metrics.percentile("train", 90).getValue().longValue() / 1_000_000f;
            logger.info(String.format("train P50: %.3f ms, P90: %.3f ms", p50, p90));

            p50 = metrics.percentile("forward", 50).getValue().longValue() / 1_000_000f;
            p90 = metrics.percentile("forward", 90).getValue().longValue() / 1_000_000f;
            logger.info(String.format("forward P50: %.3f ms, P90: %.3f ms", p50, p90));

            p50 = metrics.percentile("step", 50).getValue().longValue() / 1_000_000f;
            p90 = metrics.percentile("step", 90).getValue().longValue() / 1_000_000f;
            logger.info(String.format("step P50: %.3f ms, P90: %.3f ms", p50, p90));

            p50 = metrics.percentile("epoch", 50).getValue().longValue() / 1_000_000f;
            p90 = metrics.percentile("epoch", 90).getValue().longValue() / 1_000_000f;
            logger.info(String.format("epoch P50: %.3f ms, P90: %.3f ms", p50, p90));

            if (arguments.getOutputDir() != null) {
                MemoryUtils.dumpMemoryInfo(metrics, arguments.getOutputDir());
            }

            return true;
        } catch (ParseException e) {
            HelpFormatter formatter = new HelpFormatter();
            formatter.setLeftPadding(1);
            formatter.setWidth(120);
            formatter.printHelp(e.getMessage(), options);
        } catch (Throwable t) {
            logger.error("Unexpected error", t);
        }
        return false;
    }

    protected abstract void train(Arguments arguments) throws IOException, ModelNotFoundException;

    @Override
    public void onTrainingBatch() {
        MemoryUtils.collectMemoryInfo(metrics);
        if (trainingProgressBar == null) {
            trainingProgressBar = new ProgressBar("Training", trainDataSize);
        }
        trainingProgressBar.printProgress(trainingProgress++);
    }

    @Override
    public void onValidationBatch() {
        MemoryUtils.collectMemoryInfo(metrics);
        if (validateProgressBar == null) {
            validateProgressBar = new ProgressBar("Validating", validateDataSize);
        }
        validateProgressBar.printProgress(validateProgress++);
    }

    @Override
    public void onEpoch() {
        if (epochTime > 0L) {
            metrics.addMetric("epoch", System.nanoTime() - epochTime);
            printTrainingStatus(metrics);
        }

        epochTime = System.nanoTime();

        trainingProgress = 0;
        validateProgress = 0;
    }

    public float getTrainingAccuracy() {
        return trainingAccuracy;
    }

    public float getTrainingLoss() {
        return trainingLoss;
    }

    private void printTrainingStatus(Metrics metrics) {
        List<Metric> list = metrics.getMetric("train_Loss");
        trainingLoss = list.get(list.size() - 1).getValue().floatValue();

        list = metrics.getMetric("train_Accuracy");
        trainingAccuracy = list.get(list.size() - 1).getValue().floatValue();

        list = metrics.getMetric("validate_Loss");
        float validateLoss = list.get(list.size() - 1).getValue().floatValue();

        list = metrics.getMetric("validate_Accuracy");
        float validateAccuracy = list.get(list.size() - 1).getValue().floatValue();

        logger.info("train accuracy: {}, train loss:: {}", trainingAccuracy, trainingLoss);
        logger.info("validate accuracy: {}, validate loss: {}", validateAccuracy, validateLoss);
    }
}