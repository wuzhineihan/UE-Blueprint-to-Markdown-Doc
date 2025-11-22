// Copyright (c) 2025 A-Maze Games Website: www.a-maze.games All rights reserved.


document.addEventListener('DOMContentLoaded', function() {
    var SearchEngine = {
        state: {
            active: false,
            term: '',
            results: [],
            currentIndex: -1,
            processing: false,
            searchTimeout: null,
            options: {
                caseSensitive: false,
                wholeWords: false
            }
        },
        elements: {
            input: null,
            caseSensitive: null,
            wholeWords: null,
            counter: null,
            prevBtn: null,
            nextBtn: null,
            navControls: null
        },
        init: function() {
            this.elements.input = document.getElementById('sidebar-search-input');
            this.elements.caseSensitive = document.getElementById('search-case-sensitive');
            this.elements.wholeWords = document.getElementById('search-whole-words');
            this.elements.counter = document.getElementById('search-counter');
            this.elements.prevBtn = document.getElementById('search-prev-btn');
            this.elements.nextBtn = document.getElementById('search-next-btn');
            this.elements.navControls = document.getElementById('search-nav-controls');
            this.bindEvents();
            this.setupKeyboardShortcuts();
        },
        bindEvents: function() {
            var self = this;
            if (this.elements.input) {
                this.elements.input.addEventListener('input', function() {
                    self.handleSearchInput(this.value.trim());
                });
                this.elements.input.addEventListener('keydown', function(e) {
                    self.handleSearchKeydown(e);
                });
            }
            if (this.elements.caseSensitive) {
                this.elements.caseSensitive.addEventListener('change', function() {
                    self.updateOption('caseSensitive', this.checked);
                });
            }
            if (this.elements.wholeWords) {
                this.elements.wholeWords.addEventListener('change', function() {
                    self.updateOption('wholeWords', this.checked);
                });
            }
            if (this.elements.prevBtn) {
                this.elements.prevBtn.addEventListener('click', function() {
                    self.navigateResults('prev');
                });
            }
            if (this.elements.nextBtn) {
                this.elements.nextBtn.addEventListener('click', function() {
                    self.navigateResults('next');
                });
            }
        },
        setupKeyboardShortcuts: function() {
            var self = this;
            document.addEventListener('keydown', function(e) {
                if ((e.ctrlKey || e.metaKey) && e.key === 'f') {
                    e.preventDefault();
                    self.focusSearch();
                }
            });
        },
        handleSearchInput: function(term) {
            var self = this;
            if (this.state.searchTimeout) {
                clearTimeout(this.state.searchTimeout);
            }
            if (term.length < 2) {
                this.resetSearch();
                return;
            }
            this.state.searchTimeout = setTimeout(function() {
                self.state.term = term;
                self.state.active = true;
                self.performSearch();
            }, 300);
        },
        handleSearchKeydown: function(e) {
            if (e.key === 'Enter') {
                e.preventDefault();
                if (e.shiftKey) {
                    this.navigateResults('prev');
                } else {
                    this.navigateResults('next');
                }
            } else if (e.key === 'Escape') {
                this.clearSearch();
            }
        },
        updateOption: function(option, value) {
            this.state.options[option] = value;
            var element = this.elements[option];
            if (element && element.parentElement) {
                element.parentElement.classList.toggle('checked', value);
            }
            if (this.state.active) {
                this.performSearch();
            }
        },
        performSearch: function() {
            if (this.state.processing) return;
            this.state.processing = true;
            this.clearHighlights();
            this.state.results = [];
            this.state.currentIndex = -1;
            var pattern = this.buildSearchPattern();
            var flags = this.state.options.caseSensitive ? 'g' : 'gi';
            var searchRegex = new RegExp(pattern, flags);
            var mainContent = document.getElementById('main-content');
            if (mainContent) {
                this.searchInBatches(mainContent, searchRegex);
            } else {
                this.state.processing = false;
            }
        },
        buildSearchPattern: function() {
            var term = this.state.term;
            var escapedTerm = term.replace(/[.+?^${}()|[\]\\]/g, '\\$&');
            var wildcardPattern = escapedTerm.replace(/\\\*/g, '.*');
            if (this.state.options.wholeWords) {
                if (wildcardPattern.indexOf('.*') === 0) {
                    wildcardPattern = wildcardPattern.substring(2);
                } else {
                    wildcardPattern = '\\b' + wildcardPattern;
                }
                if (wildcardPattern.lastIndexOf('.*') === wildcardPattern.length - 2) {
                    wildcardPattern = wildcardPattern.substring(0, wildcardPattern.length - 2);
                } else {
                    wildcardPattern = wildcardPattern + '\\b';
                }
            }
            return wildcardPattern;
        },
        searchInBatches: function(rootElement, regex) {
            var self = this;
            var allTextNodes = this.getAllTextNodes(rootElement);
            var batchSize = 50;
            var currentIndex = 0;
            function processBatch() {
                var endIndex = Math.min(currentIndex + batchSize, allTextNodes.length);
                for (var i = currentIndex; i < endIndex; i++) {
                    if (!self.state.active || self.state.term !== self.elements.input.value.trim()) {
                        return;
                    }
                    self.searchTextNode(allTextNodes[i], regex);
                }
                currentIndex = endIndex;
                if (currentIndex < allTextNodes.length) {
                    setTimeout(processBatch, 10);
                } else {
                    self.finishSearch();
                }
            }
            processBatch();
        },
        getAllTextNodes: function(element) {
            var textNodes = [];
            var walker = document.createTreeWalker(
                element,
                NodeFilter.SHOW_TEXT,
                {
                    acceptNode: function(node) {
                        var parent = node.parentElement;
                        if (!parent) return NodeFilter.FILTER_REJECT;
                        if (parent.classList.contains('search-highlight') ||
                            parent.tagName === 'SCRIPT' ||
                            parent.tagName === 'STYLE') {
                            return NodeFilter.FILTER_REJECT;
                        }
                        return node.textContent.trim() ? NodeFilter.FILTER_ACCEPT : NodeFilter.FILTER_REJECT;
                    }
                },
                false
            );
            var node;
            while (node = walker.nextNode()) {
                textNodes.push(node);
            }
            return textNodes;
        },
        searchTextNode: function(textNode, regex) {
            var text = textNode.textContent;
            var match;
            var lastIndex = 0;
            regex.lastIndex = 0;
            while ((match = regex.exec(text)) !== null) {
                if (match[0].length > 0) {
                    this.highlightMatch(textNode, match.index, match[0].length);
                    break;
                }
                if (regex.lastIndex === match.index) {
                    regex.lastIndex++;
                }
            }
            regex.lastIndex = 0;
        },
        highlightMatch: function(textNode, startOffset, matchLength) {
            var parent = textNode.parentNode;
            if (!parent) return;
            var beforeText = textNode.textContent.substring(0, startOffset);
            var matchText = textNode.textContent.substring(startOffset, startOffset + matchLength);
            var afterText = textNode.textContent.substring(startOffset + matchLength);
            var fragment = document.createDocumentFragment();
            if (beforeText) {
                fragment.appendChild(document.createTextNode(beforeText));
            }
            var highlightSpan = document.createElement('span');
            highlightSpan.className = 'search-highlight';
            highlightSpan.appendChild(document.createTextNode(matchText));
            fragment.appendChild(highlightSpan);
            if (afterText) {
                fragment.appendChild(document.createTextNode(afterText));
            }
            this.state.results.push(highlightSpan);
            parent.replaceChild(fragment, textNode);
        },
        finishSearch: function() {
            this.state.processing = false;
            this.updateUI();
            if (this.state.results.length > 0) {
                this.navigateResults('first');
            }
        },
        updateUI: function() {
            if (this.elements.navControls) {
                this.elements.navControls.style.display = this.state.results.length > 0 ? 'flex' : 'none';
            }
            this.updateCounter();
        },
        updateCounter: function() {
            if (this.elements.counter) {
                if (this.state.results.length === 0) {
                    this.elements.counter.textContent = '0/0';
                } else {
                    this.elements.counter.textContent = (this.state.currentIndex + 1) + '/' + this.state.results.length;
                }
            }
        },
        navigateResults: function(direction) {
            if (this.state.results.length === 0) return;
            if (this.state.currentIndex >= 0 && this.state.currentIndex < this.state.results.length) {
                this.state.results[this.state.currentIndex].classList.remove('current');
            }
            if (direction === 'next') {
                this.state.currentIndex = (this.state.currentIndex + 1) % this.state.results.length;
            } else if (direction === 'prev') {
                this.state.currentIndex = (this.state.currentIndex - 1 + this.state.results.length) % this.state.results.length;
            } else if (direction === 'first') {
                this.state.currentIndex = 0;
            }
            var currentResult = this.state.results[this.state.currentIndex];
            currentResult.classList.add('current');
            currentResult.scrollIntoView({
                block: 'center'
            });
            this.updateCounter();
        },
        clearHighlights: function() {
            var highlights = document.querySelectorAll('.search-highlight');
            for (var i = 0; i < highlights.length; i++) {
                var highlight = highlights[i];
                var parent = highlight.parentNode;
                if (parent) {
                    var textNode = document.createTextNode(highlight.textContent);
                    parent.replaceChild(textNode, highlight);
                    parent.normalize();
                }
            }
        },
        clearSearch: function() {
            if (this.elements.input) {
                this.elements.input.value = '';
            }
            this.resetSearch();
        },
        resetSearch: function() {
            this.clearHighlights();
            this.state.active = false;
            this.state.term = '';
            this.state.results = [];
            this.state.currentIndex = -1;
            if (this.elements.navControls) {
                this.elements.navControls.style.display = 'none';
            }
            this.updateCounter();
        },
        focusSearch: function() {
            if (this.elements.input) {
                this.elements.input.focus();
                this.elements.input.select();
            }
        }
    };
    var ClipboardManager = {
        init: function() {
            this.bindMainContentEvents();
        },
        bindMainContentEvents: function() {
            var mainContent = document.querySelector('#main-content');
            if (mainContent) {
                var self = this;
                mainContent.addEventListener('click', function(e) {
                    var copyBtn = e.target.closest('.copy-btn');
                    var sectionCopyBtn = e.target.closest('.copy-section-btn');
                    if (copyBtn) {
                        self.handleRawCopy(copyBtn);
                    }
                    if (sectionCopyBtn) {
                        self.handleSectionCopy(sectionCopyBtn);
                    }
                });
            }
        },
        handleRawCopy: function(button) {
            var codeContainer = button.closest('.code-container');
            var codeElement = codeContainer ? codeContainer.querySelector('pre code') : null;
            if (!codeElement) {
                return this.showCopyError(button, 'No code');
            }
            var text = (codeElement.innerText || codeElement.textContent || '').trim();
            if (!text) {
                return this.showCopyError(button, 'No content');
            }
            this.copyToClipboard('```blueprint\n' + text + '\n```', button);
        },
        handleSectionCopy: function(button) {
            var section = button.closest('.content-section');
            if (!section) {
                return this.showCopyError(button, 'No section');
            }
            var titleEl = section.querySelector('.section-title');
            var title = titleEl ? titleEl.innerText : 'Unknown';
            var textToCopy = '### ' + title + '\n\n';
            if (button.dataset.copyTarget === 'trace') {
                var codeEl = section.querySelector('pre code');
                var code = codeEl ? codeEl.innerText.trim() : '';
                textToCopy += '```blueprint\n' + (code || '*(No execution flow)*') + '\n```';
            } else if (button.dataset.copyTarget === 'definition') {
                textToCopy += '**Inputs**\n' + this.getIoListAsMarkdown(section, 'Inputs') + '\n\n';
                var codeEl = section.querySelector('pre code');
                var code = codeEl ? codeEl.innerText.trim() : '';
                if (code && code !== 'No valid entry node found for execution flow') {
                    textToCopy += '**Execution Flow**\n```blueprint\n' + code + '\n```\n\n';
                }
                textToCopy += '**Outputs**\n' + this.getIoListAsMarkdown(section, 'Outputs');
            }
            this.copyToClipboard(textToCopy.trim(), button);
        },
        getIoListAsMarkdown: function(section, headingText) {
            var heading = null;
            var headings = section.querySelectorAll('h4');
            for (var i = 0; i < headings.length; i++) {
                if (headings[i].innerText.trim().toLowerCase() === headingText.toLowerCase()) {
                    heading = headings[i];
                    break;
                }
            }
            var ioList = heading ? heading.nextElementSibling : null;
            if (!ioList || !ioList.classList.contains('io-list')) {
                return '*(None)*';
            }
            var markdown = '';
            var items = ioList.querySelectorAll('.io-item');
            for (var i = 0; i < items.length; i++) {
                var item = items[i];
                var typeEl = item.querySelector('.io-type');
                var nameEl = item.querySelector('.io-name');
                var type = typeEl ? typeEl.innerText.trim() : '';
                var name = nameEl ? nameEl.innerText.trim() : '';
                if (type && name) {
                    markdown += '* `' + type + '` `' + name + '`\n';
                }
            }
            return markdown.trim() || '*(None)*';
        },
        copyToClipboard: function(text, button) {
            var span = button.querySelector('span');
            var originalText = span ? span.innerText : button.innerText;
            if (navigator.clipboard && navigator.clipboard.writeText) {
                var self = this;
                navigator.clipboard.writeText(text).then(function() {
                    self.showCopySuccess(button, originalText);
                }).catch(function() {
                    self.fallbackCopy(text, button, originalText);
                });
            } else {
                this.fallbackCopy(text, button, originalText);
            }
        },
        fallbackCopy: function(text, button, originalText) {
            var textArea = document.createElement('textarea');
            textArea.value = text;
            textArea.style.position = 'fixed';
            textArea.style.top = '-9999px';
            textArea.style.left = '-9999px';
            textArea.setAttribute('readonly', '');
            document.body.appendChild(textArea);
            try {
                textArea.select();
                textArea.setSelectionRange(0, textArea.value.length);
                var success = document.execCommand('copy');
                if (success) {
                    this.showCopySuccess(button, originalText);
                } else {
                    this.showCopyError(button, 'Copy failed');
                }
            } catch (e) {
                this.showCopyError(button, 'Copy error');
            } finally {
                if (textArea.parentNode) {
                    document.body.removeChild(textArea);
                }
            }
        },
        showCopySuccess: function(button, originalText) {
            var span = button.querySelector('span');
            if (span) {
                span.innerText = 'Copied!';
                span.style.color = '#98C379';
            }
            setTimeout(function() {
                if (span) {
                    span.innerText = originalText;
                    span.style.color = '';
                }
            }, 2000);
        },
        showCopyError: function(button, errorMsg) {
            var span = button.querySelector('span');
            if (span) {
                var original = span.innerText;
                span.innerText = errorMsg;
                span.style.color = '#E06C75';
                setTimeout(function() {
                    span.innerText = original;
                    span.style.color = '';
                }, 3000);
            }
        }
    };
    var NavigationManager = {
        init: function() {
            this.setupTocNavigation();
            this.setupSectionObserver();
        },
        setupTocNavigation: function() {
            var tocLinks = document.querySelectorAll('.toc-link');
            var mainContent = document.querySelector('#main-content');
            for (var i = 0; i < tocLinks.length; i++) {
                tocLinks[i].addEventListener('click', function(event) {
                    event.preventDefault();
                    var targetId = this.getAttribute('href');
                    if (targetId && mainContent) {
                        var targetElement = document.querySelector(targetId);
                        if (targetElement) {
                            var targetPosition = targetElement.offsetTop - mainContent.offsetTop;
                            mainContent.scrollTop = targetPosition;
                            if (targetElement.tagName.toLowerCase() === 'details' && !targetElement.hasAttribute('open')) {
                                var summary = targetElement.querySelector('summary');
                                if (summary) {
                                    summary.click();
                                }
                            }
                        }
                    }
                });
            }
        },
        setupSectionObserver: function() {
            var sections = document.querySelectorAll('.content-section');
            var tocLinks = document.querySelectorAll('.toc-link');
            var mainContent = document.querySelector('#main-content');
            if (sections.length && tocLinks.length && mainContent) {
                var observer = new IntersectionObserver(function(entries) {
                    entries.forEach(function(entry) {
                        var link = document.querySelector('.toc-link[href="#' + entry.target.id + '"]');
                        if (link) {
                            if (entry.isIntersecting) {
                                link.classList.add('active');
                            } else {
                                link.classList.remove('active');
                            }
                        }
                    });
                }, {
                    root: mainContent,
                    rootMargin: '-40% 0px -60% 0px'
                });
                for (var i = 0; i < sections.length; i++) {
                    observer.observe(sections[i]);
                }
            }
        }
    };
    SearchEngine.init();
    ClipboardManager.init();
    NavigationManager.init();
});
